import asyncio
import os
import re
import time
import uuid

import httpx
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel

OLLAMA_URL = os.getenv("OLLAMA_URL", "http://127.0.0.1:11434")
MODEL = os.getenv("MODEL", "qwen3.5:9b")

TOOL_SYSTEM_PROMPT = "You are a helpful assistant. Call get_nfl_scoreboard to look up NFL scores."

SCREEN_SYSTEM_PROMPT = (
    "You are a tiny desk companion shown on a 2.8 inch screen. "
    "Answer using only the provided scoreboard data. "
    "Reply in plain text, under 25 words, no markdown, no emoji."
)

DEFAULT_PROMPT = "What is the current score of the Buffalo Bills and Kansas City Chiefs game?"
MAX_TOKENS = 80

TOOLS = [
    {
        "type": "function",
        "function": {
            "name": "get_nfl_scoreboard",
            "description": "Fetch live NFL game scores and game status from ESPN.",
            "parameters": {
                "type": "object",
                "properties": {},
                "required": [],
            },
        },
    }
]

app = FastAPI()
client = httpx.AsyncClient(timeout=300)
gpu_lock = asyncio.Lock()
jobs: dict[str, dict] = {}


class AskBody(BaseModel):
    prompt: str | None = None


async def fetch_espn_scores() -> str:
    url = "https://site.web.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard"
    headers = {
        "User-Agent": "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36",
        "Accept": "application/json",
    }
    r = await client.get(url, headers=headers, follow_redirects=False)
    print(f"[ESPN HTTP STATUS]: {r.status_code}, url={r.url}, length={len(r.content)}")

    try:
        data = r.json()
    except Exception:
        print(f"[ESPN RAW RESPONSE ERROR]: {r.text[:300]}")
        return "ESPN API did not return valid JSON."

    games = []
    for event in data.get("events", []):
        competitions = event.get("competitions", [])
        if not competitions:
            continue
        comp = next(iter(competitions))
        status = comp.get("status", {}).get("type", {}).get("shortDetail", "")
        teams = [
            f"{c['team']['displayName']}: {c.get('score', '0')}"
            for c in comp.get("competitors", [])
        ]
        games.append(f"{' vs '.join(teams)} ({status})")

    result = "\n".join(games) if games else "No NFL games scheduled right now."
    print(f"[ESPN TOOL OUTPUT]\n{result}\n")
    return result


async def run_model(prompt: str) -> str:
    messages = [
        {"role": "system", "content": TOOL_SYSTEM_PROMPT},
        {"role": "user", "content": prompt},
    ]

    async with gpu_lock:
        # Step 1: Allow thinking + neutral prompt so Qwen triggers tool_calls reliably
        r = await client.post(
            f"{OLLAMA_URL}/api/chat",
            json={
                "model": MODEL,
                "stream": False,
                "think": True,
                "keep_alive": "30m",
                "tools": TOOLS,
                "options": {"num_predict": 512, "temperature": 0.1},
                "messages": messages,
            },
        )
        r.raise_for_status()
        msg = r.json()["message"]
        print(f"[OLLAMA STEP 1]: {msg}")

        # Step 2: Execute the tool if requested
        if msg.get("tool_calls"):
            messages.append(msg)

            for tool in msg["tool_calls"]:
                if tool["function"]["name"] == "get_nfl_scoreboard":
                    tool_result = await fetch_espn_scores()
                    messages.append({
                        "role": "tool",
                        "content": tool_result,
                    })

            # Replace system prompt with the 2.8-inch screen persona
            messages.pop(0)
            messages.insert(0, {"role": "system", "content": SCREEN_SYSTEM_PROMPT})

            # Step 3: Fast non-thinking final answer under 25 words
            r = await client.post(
                f"{OLLAMA_URL}/api/chat",
                json={
                    "model": MODEL,
                    "stream": False,
                    "think": False,
                    "keep_alive": "30m",
                    "options": {"num_predict": MAX_TOKENS, "temperature": 0.5},
                    "messages": messages,
                },
            )
            r.raise_for_status()
            msg = r.json()["message"]

    text = msg.get("content", "")
    text = re.sub(r"<think>.*?</think>", "", text, flags=re.DOTALL)
    return text.strip()


@app.get("/health")
async def health():
    return {"ok": True, "model": MODEL}


@app.post("/chat")
async def chat(body: AskBody | None = None):
    prompt = body.prompt if body and body.prompt else DEFAULT_PROMPT
    try:
        return {"reply": await run_model(prompt)}
    except httpx.HTTPError as e:
        raise HTTPException(502, f"API error: {e}")


@app.post("/jobs")
async def create_job(body: AskBody | None = None):
    prompt = body.prompt if body and body.prompt else DEFAULT_PROMPT
    job_id = uuid.uuid4().hex[:8]
    jobs[job_id] = {"status": "pending", "reply": None, "created": time.time()}

    async def work():
        try:
            jobs[job_id].update(status="done", reply=await run_model(prompt))
        except Exception as e:
            jobs[job_id].update(status="error", reply=str(e))

    asyncio.create_task(work())
    return {"id": job_id}


@app.get("/jobs/{job_id}")
async def get_job(job_id: str):
    job = jobs.get(job_id)
    if not job:
        raise HTTPException(404, "unknown job")
    return job

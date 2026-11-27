# SCHUNET — Smoke Checks

**Purpose:** Manual checks that stand in for an automated test suite during the PoC (ADR 004). Run the relevant section after applying a step's delta zip, and before committing.
**Rules:** Every step that changes behavior adds or updates its section here. Record results in your commit message or `CURRENT_SESSION.md` if something is surprising. AI assistants usually cannot run these (no Ollama, no network); they must write the checks and say they were not run.

Assumptions: server started from `server/` with `uvicorn server:app --host 0.0.0.0 --port 8000`, Ollama running locally, `BASE=http://127.0.0.1:8000`. Local CPU inference can take minutes per request; expect to wait.

## Baseline (Step 1) — current behavior of the original prototype

These describe what the code did at the start of the milestone. They may change later per ADR 004; update this section when they do.

1. **Install and start.** Fresh venv, `pip install -r requirements.txt`, then start the server. Expect it to start with no errors.
2. **Health.** `curl -s $BASE/health`. Expect JSON containing `ok: true` and the configured `model`.
3. **Chat with default prompt.** `curl -s -X POST $BASE/chat -H 'Content-Type: application/json' -d '{}'`. Expect JSON with a `reply` string (the default prompt asks about an NFL game, so the ESPN tool is likely called; the reply should be under about 25 words).
4. **Chat with a prompt.** `curl -s -X POST $BASE/chat -H 'Content-Type: application/json' -d '{"prompt":"Say hello in five words."}'`. Expect a `reply` string.
5. **Jobs.** `curl -s -X POST $BASE/jobs -d '{}' -H 'Content-Type: application/json'` returns an `id`; `curl -s $BASE/jobs/<id>` reports `pending`, then `done` (or `error`) with a `reply`. An unknown id returns 404.
6. **Upstream failure.** Stop Ollama, call `/chat`. Expect an HTTP error response (the original code returned 502), not a hang or a crash; the server keeps running.

## Added by later steps

Each step appends its own section below (for example: configuration validation cases, queue behavior, tool-registry errors, event rows in SQLite, monitoring API/auth, UI manual checklist). Keep each check to one action and one expected result.

# SCHUNET — Architecture

**Status:** Describes the current prototype and records the intended architectural boundaries.  
**Last reviewed:** 2026-11-27

## 1. Current system at a glance

```text
┌──────────────────────────────┐
│ ESP32 Codec client           │
│ 2.7-inch TFT / touch input   │
└──────────────┬───────────────┘
               │ Wi-Fi HTTP
               │ POST /chat
               ▼
┌──────────────────────────────┐
│ Python FastAPI bridge        │
│ server/server.py             │
│                              │
│ - /health                    │
│ - /chat                      │
│ - /jobs                      │
│ - /jobs/{job_id}             │
│ - ESPN tool implementation   │
└──────────────┬───────────────┘
               │ HTTP /api/chat
               ▼
┌──────────────────────────────┐
│ Ollama                       │
│ qwen3.5:9b by default        │
└──────────────────────────────┘
               │
               └── ESPN scoreboard API (when tool called)
```

The firmware includes `schunet_logo.h`, while the checked-in header in the supplied snapshot is named `SCHUNET_logo.h`; the case mismatch should be resolved deliberately for case-sensitive filesystems.

The ESP32 does not run inference. It sends an HTTP request to the Python service, which calls Ollama and may call the ESPN API before returning a compact text response.

## 2. Current components

### 2.1 ESP32 client

Source: `schunet-client-2.7/schunet-client-2.7.ino`

Responsibilities visible in the firmware and existing project notes:

- Connect to Wi-Fi using values supplied through the local secrets header.
- Render the persistent header, navigation, home screen, AI Agent screen, and temporary countdown screen.
- Handle touch input and serial shortcuts.
- Start the HTTP request from a FreeRTOS task so the main UI loop can continue animating.
- Send a request to `CHAT_URL`, currently constructed as `/chat`.
- Parse the server's `reply` field and animate the text locally.

The microphone icon is currently a request trigger; there is no microphone capture or speech-to-text pipeline in the firmware.

### 2.2 FastAPI bridge

Source: `server/server.py`

Responsibilities:

- Define the HTTP API consumed by the ESP32 and usable by other clients.
- Call Ollama using `httpx.AsyncClient`.
- Serialize model generation through an `asyncio.Lock` (`gpu_lock` in the current source).
- Expose a synchronous chat endpoint and an in-memory background-job pattern.
- Execute the one registered ESPN scoreboard tool when the model requests it.

The current implementation is a single module. The boundaries in this document are logical descriptions, not separate deployable services.

### 2.3 Model integration

The server currently integrates directly with Ollama's native `/api/chat` interface. `OLLAMA_URL` and `MODEL` are environment-configurable. There is no provider-neutral interface or cloud provider adapter yet.

When a tool call is returned, the server executes `get_nfl_scoreboard`, appends a tool result, then asks the model for a short screen-friendly response. The code currently contains hard-coded system prompts and tool definitions for this single use case.

### 2.4 Tool integration

The only implemented tool is `get_nfl_scoreboard`, which calls ESPN's NFL scoreboard endpoint and condenses the response into text. The `TOOLS` list and conditional dispatch in `run_model()` are currently hard-coded; this is not yet a general-purpose tool registry.

## 3. Current API contract

| Route | Method | Current behavior |
|---|---|---|
| `/health` | GET | Returns `ok` and configured model name. |
| `/chat` | POST | Accepts an optional `prompt`; returns `{"reply": "..."}`. If no prompt is supplied, uses `DEFAULT_PROMPT`. |
| `/jobs` | POST | Creates an in-memory background task and returns a short job ID. |
| `/jobs/{job_id}` | GET | Returns the in-memory job record, including status and reply. |

Important implementation detail: the current ESP32 firmware calls `/chat` with an empty JSON object. Although `/jobs` exists on the server, the firmware does not currently use it.

The job store is an in-memory Python dictionary. Jobs are not durable across server restarts and there is no cleanup/expiration mechanism in the current code.

## 4. Intended target boundaries

These are proposed logical modules for gradual refactoring, not existing modules:

- **Client/interface layer:** ESP32, desktop, web, and future devices.
- **Core API:** stable request/response contracts, sessions, device identity, and transport concerns.
- **Orchestration:** model selection, task lifecycle, context assembly, and agent execution.
- **Provider adapters:** a common interface implemented by Ollama and future hosted providers.
- **Capability/tool registry:** independently registered functions and integrations callable under explicit policy.
- **Infrastructure:** configuration, persistence, secrets, logs, metrics, and access control.

Keep these as modules within the existing Python application initially. Do not split them into separate services without a concrete scaling or deployment reason.

## 5. Architectural constraints and risks

- The current model host is resource-constrained relative to larger models; avoid unnecessary always-running services.
- The ESP32 needs a small, stable, predictable response format and should not need provider-specific logic.
- The current firmware expects a `reply` string and has a small display area; richer responses will need deliberate presentation rules.
- The server binds to `0.0.0.0` in the documented run command so the ESP32 can reach it. The current source does not implement authentication or authorization. Treat network exposure and tool permissions as security work before expanding access.
- The current server creates a module-level `httpx.AsyncClient`; an explicit application lifespan/close path is not currently present.
- Tool execution is hard-coded and should not be generalized until a small, explicit registry and permission model are designed.
- Do not assume the README or project notes are always synchronized with source; verify behavior in code before changing contracts.

## 6. Design rules for future changes

1. Keep device contracts independent of model vendor and model-specific response formats.
2. Keep provider-specific request/response translation inside provider adapters.
3. Keep tools separate from provider adapters; tools are capabilities, not models.
4. Treat routing as a policy decision informed by capability, latency, cost, privacy, and local resource availability.
5. Make long-running work observable and cancellable where practical; define job lifecycle and retention before relying on the current in-memory prototype.
6. Add tests around contracts and routing before adding more providers or devices.
7. Preserve the current ESP32 `/chat` behavior during the first backend refactoring unless a deliberate, tested migration is planned.

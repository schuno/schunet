# SCHUNET — Architecture

**Status:** Describes the current prototype and records the intended target boundaries.  
**Last reviewed:** 2026-11-27

## 1. Current system at a glance

```text
ESP32 Codec client (2.7-inch TFT)
          │ Wi-Fi HTTP / POST /chat
          ▼
Python FastAPI bridge (server/server.py)
          │ HTTP /api/chat
          ▼
Ollama (qwen3.5:9b by default)
          │
          └── ESPN scoreboard API (when tool called)
```

The firmware includes `schunet_logo.h`, while the checked-in header in the supplied snapshot is named `SCHUNET_logo.h`; the case mismatch should be resolved deliberately for case-sensitive filesystems.

The ESP32 does not run inference. It sends an HTTP request to the Python service, which calls Ollama and may call the ESPN API before returning a compact text response.

## 2. Current components

### 2.1 ESP32 client

Source: `schunet-client-2.7/schunet-client-2.7.ino`

Responsibilities include Wi-Fi setup, TFT rendering, touch/serial navigation, asynchronous HTTP requests, and local typewriter presentation. The microphone icon is currently a request trigger; there is no microphone capture or speech-to-text pipeline.

### 2.2 FastAPI bridge

Source: `server/server.py`

The server exposes the device/client API, calls Ollama through `httpx.AsyncClient`, serializes model generation through an `asyncio.Lock`, exposes synchronous chat and in-memory background jobs, and executes the ESPN tool when requested. The implementation is currently a single module; future boundaries in this document are logical modules, not separate services.

### 2.3 Model and tool integration

The server directly uses Ollama's native `/api/chat` interface. `OLLAMA_URL` and `MODEL` are environment-configurable. The ESPN tool schema, system prompts, and dispatch are hard-coded. There is no provider-neutral contract, hosted adapter, general tool registry, or model router yet.

## 3. Current API contract

| Route | Method | Current behavior |
|---|---|---|
| `/health` | GET | Returns `ok` and configured model name. |
| `/chat` | POST | Accepts an optional `prompt`; returns `{"reply": "..."}`. If no prompt is supplied, uses `DEFAULT_PROMPT`. |
| `/jobs` | POST | Creates an in-memory background task and returns a short job ID. |
| `/jobs/{job_id}` | GET | Returns the in-memory job record, including status and reply. |

The current ESP32 calls `/chat` with an empty JSON object. The `/jobs` store is an in-memory Python dictionary, not durable and without cleanup/expiration.

## 4. Intended target architecture

```text
                         ┌─────────────────────────────┐
                         │ Monitoring / Control UI     │
                         │ tasks, agents, events, usage│
                         └──────────────┬──────────────┘
                                        │ read/control API
┌─────────────────────┐                 ▼
│ ESP32 / future      │       ┌──────────────────────────────┐
│ device clients      ├──────►│ Core API + Organization     │
└─────────────────────┘       │ departments, roles, tasks,   │
                              │ assignment, review, policy  │
                              └──────────────┬───────────────┘
                                             ▼
                              ┌──────────────────────────────┐
                              │ Orchestration runtime       │
                              │ decomposition, context,     │
                              │ execution, approvals, events│
                              └──────┬───────────┬───────────┘
                                     │           │
                          ┌──────────▼───┐  ┌────▼────────────┐
                          │ Model router │  │ Capability/tool │
                          │ + policies   │  │ registry        │
                          └──────┬───────┘  └─────────────────┘
                                 ▼
                       ┌──────────────────────┐
                       │ Provider adapters    │
                       │ Ollama / hosted APIs │
                       └──────────────────────┘
```

All components remain modules in one Python application initially. The diagram describes responsibilities and dependencies, not a microservice deployment.

### 4.1 Organization and agent definitions

The organization layer defines departments, roles, agent profiles, responsibilities, instructions, allowed tools, and default routing preferences. It also owns task assignment, dependencies, delegation, and review.

An agent is not synonymous with a model. The same agent can use different models for different tasks, and multiple agents can use the same model profile. A role's default routing profile is a fallback or preference—not a hard binding.

### 4.2 Task lifecycle and decomposition

The orchestration runtime should turn a request into structured tasks before model selection for execution. A task should record at least:

- Goal, description, acceptance criteria, and relevant context references.
- Assigned agent/role and any dependencies.
- Complexity/effort estimate, required capabilities, and risk level.
- Privacy constraints and any approval requirements.
- Status, timestamps, attempts, result, and review outcome.

The director/planner may use a capable planning model to decompose a broad request. Each resulting task is then evaluated independently by the router. Re-evaluation is allowed when new evidence changes the task's scope, but routing decisions must be recorded.

### 4.3 Configuration-driven model routing

Configuration should define provider connections, model aliases/profiles, and routing policies. Do not scatter vendor model IDs or provider-specific parameters through agent code.

A model profile should describe, where known:

- Provider and model identifier (kept in configuration, not role prompts).
- Supported capabilities and tool/function-call support.
- Context/output limits.
- Relative reasoning/coding capability or a deliberately small capability tier.
- Cost metadata, latency expectations, and availability constraints.
- Privacy eligibility and whether the model is local or hosted.

A routing policy evaluates a decomposed task using:

1. Task complexity/reasoning demand.
2. Required capabilities and context size.
3. Risk/consequence and required confidence/review level.
4. Data privacy and provider eligibility.
5. Estimated cost, latency, provider availability, and local resource pressure.

For example, a straightforward implementation task may use a lower-cost profile, while architecture, ambiguous debugging, security-sensitive changes, or difficult review may use a stronger reasoning profile. The router should not assume that “junior” always means one model or “senior” always means another: the task's requirements take precedence, with role defaults used only when the task lacks stronger routing signals.

Start with explicit, deterministic rules and a small number of complexity tiers (for example: routine, standard, demanding, critical). Add learned/scored routing only after telemetry and evaluation data exist. If no eligible model is available, fail clearly or use an explicitly configured fallback; never silently incur hosted/paid usage.

Cloudflare AI Gateway should be represented as a gateway/observability path where configured. It is not itself a model identity: configuration must still identify the actual inference provider and model behind the gateway. Keep the gateway optional so local Ollama remains usable independently.

### 4.4 Provider adapters

Define a minimal provider-neutral request/result contract and implement Ollama first. Add a hosted adapter using the user's tested Cloudflare Gateway path only after capturing its actual endpoint/auth/request shape in local configuration. Normalize response text, tool calls (if supported), usage, latency, and provider/model metadata. Usage/cost fields may be unavailable and must be represented as unknown rather than fabricated.

### 4.5 Capability/tool registry and permissions

Tools are capabilities, not providers. Register tools independently with schemas, descriptions, execution handlers, and permission requirements. Validate model-supplied tool names and arguments. Apply per-agent/per-task allowlists and approval gates. Do not expose unrestricted shell or filesystem access by default.

### 4.6 Monitoring and event model

The monitoring UI is an initial product requirement. The backend should emit structured lifecycle events for task creation, decomposition, assignment, routing decisions, model execution, tool invocation, approval/waiting, retries, completion, failure, and review.

Persist task records and operational events from the first useful vertical slice. SQLite is a reasonable initial local persistence option, but the storage technology remains a decision to confirm before implementation. This persistence is for operational history and observability; it does not require implementing general conversational memory at the same time.

The initial UI should show a live/current view and allow inspection of prior task runs. Capture provider/model, routing reason, timing, token counts when returned, estimated cost when computable, tool calls, errors, and local resource information when available. Clearly distinguish missing telemetry from zero usage/cost.

### 4.7 Future animated office

The animated office is a future visualization of the same organization and event data. Agent position/animation should be derived from real states (idle, assigned, executing, waiting for approval, reviewing, completed). The visualization is decorative and must not own task state, scheduling, or control flow. Build the monitoring data model first so the visual layer can be added without changing orchestration.

## 5. Initial configuration shape (illustrative)

The exact file format and schema are not yet selected. Keep secrets in environment variables or a local ignored secrets file, never in committed config or prompts. A future configuration may look conceptually like this:

```toml
[providers.ollama]
enabled = true
base_url = "http://127.0.0.1:11434"

[providers.cloudflare_gateway]
enabled = false
# credentials and gateway URL supplied outside committed config

[models.local_fast]
provider = "ollama"
model = "<configured-local-model>"
capabilities = ["general", "basic-coding"]

[models.hosted_reasoning]
provider = "cloudflare_gateway"
model = "<configured-provider-model>"
capabilities = ["reasoning", "coding", "tool-use"]

[routing.defaults]
policy = "task_complexity_v1"
allow_paid_fallback = false

[routing.task_complexity_v1]
routine = "local_fast"
standard = "local_fast"
demanding = "hosted_reasoning"
critical = "hosted_reasoning"
```

This is a conceptual example, not a committed schema or a recommendation to use these exact aliases/model assignments. The initial policy must be configurable, validate aliases at startup, and record its decision/reason for each task.

## 6. Security, resource, and compatibility constraints

- The current model host is resource-constrained; agent concurrency must not imply unrestricted concurrent local inference. Use a bounded execution queue/semaphore for Ollama and make concurrency configurable.
- The ESP32 needs a small, stable response format and should not know about provider/model details. Preserve the existing `/chat` `reply` contract during initial backend refactoring.
- The server binds to `0.0.0.0` in the documented run command and currently has no authentication/authorization. Add access control before exposing organization tools or control actions beyond a trusted local network.
- Keep API tokens and credentials outside source control and model context. Do not put secrets in logs or monitoring events.
- Scope tools by agent/task; require human approval for consequential or destructive operations (for example, arbitrary shell commands, installs, file deletion, Git push, or external side effects).
- Make long-running tasks observable, cancellable where practical, and bounded by time/step/retry limits.
- Do not confuse Cloudflare AI Gateway logging with application-level task/agent observability; SCHUNET still needs its own task and event records.
- The current server creates a module-level `httpx.AsyncClient`; introduce an explicit application lifespan/close path during refactoring.

## 7. Design rules

1. Keep device contracts independent of model vendor and model-specific response formats.
2. Keep provider-specific translation inside provider adapters.
3. Keep tools separate from providers; enforce explicit capability permissions.
4. Decompose work before routing execution tasks; choose models per task, not solely per agent title.
5. Keep routing policies and model aliases configuration-driven and auditable.
6. Treat monitoring/event capture as foundational; build visualizations on top of real state.
7. Keep the animated office optional and non-authoritative.
8. Add tests around provider contracts, routing decisions, task lifecycle, and API schemas.
9. Preserve the current ESP32 `/chat` behavior unless a deliberate, tested migration is planned.

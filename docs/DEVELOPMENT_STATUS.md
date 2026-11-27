# SCHUNET — Development Status

**Last reviewed:** 2026-11-27  
**Source of truth:** Supplied repository snapshot; verify against the working tree before relying on this file.

## Current baseline

SCHUNET is an early working prototype. The repository contains ESP32 firmware, a single-file FastAPI bridge, project artwork, a 3D-printed enclosure model, and a ZIP utility.

## Implemented in the repository

### ESP32 client

- [x] ESP32 Wi-Fi client setup.
- [x] 240×320 TFT Codec-style UI using `TFT_eSPI`.
- [x] Home, AI Agent, and temporary Release Countdown screens.
- [x] Touch navigation and serial navigation shortcuts.
- [x] Wi-Fi connection indicator/globe animation.
- [x] Asynchronous HTTP request task and pending-state animation.
- [x] Reply parsing and on-device typewriter presentation.
- [x] Agent portrait eye and mouth animation.

### Server

- [x] FastAPI application in `server/server.py`.
- [x] Ollama `/api/chat` integration using configurable `OLLAMA_URL` and `MODEL`.
- [x] `/health`, `/chat`, `/jobs`, and `/jobs/{job_id}` endpoints.
- [x] One ESPN NFL scoreboard tool, `get_nfl_scoreboard`.
- [x] Two-pass model flow when the tool is requested: tool selection/execution, then short response formatting.
- [x] In-memory background job implementation.

### Project assets and utilities

- [x] SCHUNET logo assets.
- [x] `designs/case_v2.stl` enclosure model.
- [x] `util/repo_zip.py` initial repository ZIP script.

## Verified source facts and caveats

- The firmware constructs `CHAT_URL` with `/chat` and posts an empty JSON object. It does not currently send a user-entered prompt.
- The server's `/chat` endpoint therefore falls back to `DEFAULT_PROMPT` unless another client supplies a prompt.
- The server implements `/jobs`, but the ESP32 firmware currently uses `/chat`, not `/jobs`.
- The microphone-shaped UI control triggers a request; the inspected firmware does not implement audio capture.
- The server integrates directly with Ollama. No hosted provider adapters or model router are present.
- The only tool registered in the server is the ESPN NFL scoreboard tool.
- No automated test suite, dependency manifest, or formal build configuration was present in the supplied repository ZIP.
- The supplied ZIP does not include Git metadata, so branch, commit, and working-tree status cannot be established from that snapshot.

## Not implemented yet

### Intelligence and organization

- [ ] Provider-neutral model interface and normalized response type.
- [ ] Hosted provider adapter using the tested Cloudflare AI Gateway path.
- [ ] Configurable provider/model catalog and validation.
- [ ] Task decomposition and task-level complexity classification.
- [ ] Config-driven model routing with auditable reasons and explicit paid-use controls.
- [ ] General-purpose agent runtime, organization/department/role definitions, and multi-agent delegation.
- [ ] General-purpose tool registry and per-agent/task permission policy.

### Monitoring and persistence

- [ ] Structured task/execution event model.
- [ ] Persistent operational task/event history.
- [ ] Monitoring/control UI available from the first organization workflow.
- [ ] Provider/model latency, token usage, cost, errors, and local resource telemetry where available.
- [ ] Animated office visualization (future, optional presentation layer).

### Platform and clients

- [ ] Persistent conversational memory.
- [ ] Authentication and authorization for API/device/control access.
- [ ] Automated tests and continuous integration.
- [ ] Desktop/web control interface (monitoring UI is now an initial requirement; exact UI technology is undecided).
- [ ] Voice capture and speech-to-text.

## Known implementation limitations

- `DEFAULT_PROMPT`, system prompts, tool schema, and screen formatting are hard-coded in `server/server.py`.
- The server has no provider abstraction; application logic is coupled to Ollama's API shape.
- Tool dispatch is a conditional for one named tool, not a registry.
- Background jobs are stored in process memory and are not durable.
- The current API has no authentication or authorization.
- The ESP32 receives the complete response before its typewriter animation; HTTP token streaming is not implemented.
- The display truncates long responses according to the firmware's dialogue layout.
- The countdown screen is explicitly documented as temporary, with a target date of 2026-12-02.

## Agreed planning direction (not yet implemented)

- The virtual organization begins with an IT department and a small set of roles.
- Tasks are decomposed before execution-model selection; the model router chooses per task using complexity, capability, risk, privacy, cost, latency, and availability.
- Agent roles may have default model profiles, but task requirements override role defaults.
- Model aliases, provider settings, and routing policies are configuration-driven; credentials remain outside committed configuration.
- Monitoring and structured operational history are foundational to the first organization workflow.
- The animated office is a later visualization driven by actual agent/task state.
- Keep the application a modular monolith initially and preserve the ESP32 `/chat` contract.

## Immediate next engineering sequence

1. Add a minimal test/dependency baseline and capture the existing `/health` and `/chat` behavior.
2. Introduce validated application configuration and a provider-neutral model contract; move current Ollama behavior behind an adapter.
3. Extract ESPN behind a small tool registry and permission boundary.
4. Add structured model execution results and events (provider/model, routing metadata, latency, usage where available, outcome/error).
5. Build the smallest useful monitoring UI against real backend state before expanding the agent organization.
6. Add task lifecycle/decomposition and a deterministic, config-driven complexity router; test routing independently of live provider calls.
7. Add the first hosted adapter through the user's tested Cloudflare Gateway path, with explicit enablement and no implicit paid fallback.


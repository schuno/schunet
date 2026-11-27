# SCHUNET — Development Status

**Last reviewed:** 2026-11-27  
**Source of truth:** Current repository snapshot; verify against the working tree before relying on this file.

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

## Verified source facts vs. project-note claims

- The firmware constructs `CHAT_URL` with the `/chat` path and posts an empty JSON object. It does not currently send a user-entered prompt.
- The server's `/chat` endpoint therefore falls back to `DEFAULT_PROMPT` unless another client supplies a prompt.
- The server implements `/jobs`, but the ESP32 firmware currently uses `/chat`, not `/jobs`.
- The microphone-shaped UI control triggers a request; the inspected firmware does not implement audio capture.
- The server integrates directly with Ollama. No hosted provider adapters or model router are present.
- The only tool registered in the server is the ESPN NFL scoreboard tool.
- No automated test suite, dependency manifest, or formal build configuration was present in the supplied repository ZIP.
- The supplied ZIP does not include Git metadata, so branch, commit, and working-tree status cannot be established from that snapshot.

## Not implemented yet

- [ ] Provider-neutral model interface.
- [ ] Hosted provider adapters (OpenAI, Anthropic, Gemini, or others).
- [ ] Hybrid model router or task-based model selection.
- [ ] General-purpose tool registry and tool permission policy.
- [ ] General agent runtime or multi-agent orchestration.
- [ ] Persistent sessions or memory.
- [ ] Desktop or web control interface.
- [ ] Voice capture and speech-to-text.
- [ ] Authentication and authorization for API/device access.
- [ ] Automated tests and continuous integration.
- [ ] Structured telemetry for model latency, token use, and cost.

## Known limitations

- `DEFAULT_PROMPT`, system prompts, tool schema, and screen formatting are hard-coded in `server/server.py`.
- The server has no provider abstraction; application logic is coupled to Ollama's API shape.
- Tool dispatch is a conditional for one named tool, not a registry.
- Background jobs are stored in process memory and are not durable.
- The current API has no authentication or authorization.
- The ESP32 receives the complete response before its typewriter animation; HTTP token streaming is not implemented.
- The display truncates long responses according to the firmware's dialogue layout.
- The countdown screen is explicitly documented as temporary, with a target date of 2026-12-02.

## Immediate recommended work

1. Establish and maintain this documentation/handoff workflow.
2. Add a small automated test baseline around the current server endpoints and model/tool orchestration boundaries.
3. Refactor the current Ollama integration behind a minimal provider interface without changing the ESP32 `/chat` contract.
4. Extract the ESPN integration behind a minimal tool registration/dispatch boundary.
5. Add configuration validation and safe secret handling.
6. Add one hosted provider adapter and a deliberately simple explicit provider-selection option before attempting automatic routing.

# SCHUNET — Roadmap

This roadmap separates near-term engineering work from longer-term platform ideas. Items are proposals unless explicitly marked as existing behavior in `DEVELOPMENT_STATUS.md`.

## Phase 0 — Project continuity and baseline

- [x] Establish canonical project documentation under `docs/`.
- [x] Define shared instructions and provider-independent handoff format.
- [ ] Add a context-packaging utility for AI sessions.
- [ ] Establish a minimal automated test suite.
- [ ] Record baseline API behavior and test the current ESP32 integration.

## Phase 1 — Modular backend foundation

- [ ] Extract configuration and settings validation.
- [ ] Define a small provider-neutral interface for chat/model requests.
- [ ] Move current Ollama behavior behind the first provider adapter.
- [ ] Extract ESPN scoreboard functionality behind a tool interface/registry.
- [ ] Preserve the existing `/chat` response contract for the ESP32.
- [ ] Add tests for provider translation, tool dispatch, and API schemas.

## Phase 2 — Hybrid model access

- [ ] Add one hosted provider adapter.
- [ ] Allow explicit provider/model selection per request.
- [ ] Normalize provider responses into a common internal result shape.
- [ ] Capture provider/model, latency, token usage when available, and estimated cost where applicable.
- [ ] Add a simple policy-based `auto` mode only after explicit selection works reliably.

## Phase 3 — Developer experimentation interface

- [ ] Build a lightweight desktop or web control surface.
- [ ] Inspect configured providers and available local models.
- [ ] Submit prompts and compare outputs.
- [ ] Inspect tool calls, execution results, errors, and timing.
- [ ] Display local system resource use and hosted API usage where available.

## Phase 4 — Agent and context capabilities

- [ ] Define task and session lifecycle.
- [ ] Add bounded multi-step tool execution.
- [ ] Add explicit tool permissions and approval checkpoints.
- [ ] Add persistent conversation/context storage if needed.
- [ ] Evaluate model delegation using concrete workflows before generalizing.

## Phase 5 — Device ecosystem

- [ ] Stabilize a versioned device API contract.
- [ ] Add device identity and authentication.
- [ ] Add device capability discovery.
- [ ] Explore voice, e-ink, sensors, actuators, and home automation as separate modules.

## Time-sensitive existing feature

The firmware and original project notes describe the Release Countdown screen as temporary, targeting 8:00 AM EST on 2026-12-02. Decide whether to remove it after that date; do not remove it automatically as part of unrelated refactoring.

# SCHUNET — Roadmap

This roadmap separates current implementation from planned work. The virtual organization and its monitoring UI are the next major product direction. Preserve the existing ESP32 client while evolving the backend as a modular monolith.

## Phase 0 — Project continuity and baseline

- [x] Establish canonical project documentation under `docs/`.
- [x] Define shared instructions and provider-independent handoff format.
- [ ] Verify and align the context-packaging utility, README command, and `util/README.md` against the actual working tree.
- [ ] Add a minimal automated test suite and dependency manifest.
- [ ] Record baseline `/health` and `/chat` behavior with tests, including the ESP32-compatible `reply` shape.
- [ ] Confirm the working tree and branch before implementation; the supplied ZIP has no Git metadata.

## Phase 1 — Configurable intelligence foundation

Goal: make model execution provider-neutral, configurable, and observable without changing the ESP32 contract.

- [ ] Establish validated application configuration (provider definitions, model aliases, routing policy, limits, and feature flags).
- [ ] Keep secrets in environment variables or a local ignored secrets file; validate required settings at startup.
- [ ] Define a small provider-neutral request/result contract.
- [ ] Move current Ollama behavior behind the first provider adapter.
- [ ] Define model profiles/capability metadata without hard-coding provider model IDs in agent code.
- [ ] Extract ESPN scoreboard functionality behind a small tool registry/dispatch boundary.
- [ ] Preserve the existing `/chat` response contract for the ESP32.
- [ ] Add tests for config validation, provider translation, tool dispatch, and API schemas.

## Phase 2 — Monitoring foundation (built alongside the first organization workflow)

Goal: make execution visible from the beginning, rather than adding monitoring after the agent runtime.

- [ ] Define structured task/execution events and a common event vocabulary.
- [ ] Persist task/run/event history using a lightweight local store (evaluate SQLite first; confirm before implementation).
- [ ] Capture provider/model, routing reason, timestamps, duration, token usage when available, estimated cost when available, tool activity, errors, and approval/waiting states.
- [ ] Clearly represent unavailable telemetry as unknown; do not treat missing usage/cost as zero.
- [ ] Build a minimal web monitoring/control UI that reads real backend state.
- [ ] Show task list/detail, agent roster/status, execution timeline, model/provider use, and failures.
- [ ] Keep UI technology and visual design lightweight; avoid building the animated office before the data model is stable.

## Phase 3 — Task lifecycle and complexity-based model routing

Goal: decompose requests into independently routable work units.

- [ ] Define task, dependency, assignment, attempt, review, and lifecycle states.
- [ ] Add task decomposition with acceptance criteria and bounded scope.
- [ ] Represent task complexity/effort, required capabilities, risk, context needs, and privacy constraints.
- [ ] Implement a deterministic policy router with a small set of complexity tiers (for example routine, standard, demanding, critical).
- [ ] Select a model per task using task requirements first; use agent-role defaults only as fallback/preferences.
- [ ] Record selected profile, actual provider/model, policy version, and human-readable routing reason for every execution.
- [ ] Add bounded retries, timeouts, step limits, and a configurable local inference queue.
- [ ] Test routing decisions with synthetic tasks, independent of external provider availability.

## Phase 4 — Hosted inference and hybrid routing

- [ ] Add one hosted provider adapter using the already-tested Cloudflare AI Gateway path; verify the actual underlying inference provider/model configuration.
- [ ] Keep gateway configuration distinct from provider/model identity.
- [ ] Allow explicit provider/model selection per task and normalize responses.
- [ ] Make hosted usage explicitly enabled and observable; never silently route to a paid model.
- [ ] Add token/cost metadata where the provider returns enough information; mark unavailable values as unknown.
- [ ] Add configured fallbacks and provider availability handling.
- [ ] Introduce policy-based `auto` selection only after explicit selection and deterministic routing are reliable.

## Phase 5 — IT department and bounded agent execution

- [ ] Define organization, department, role, and agent configuration.
- [ ] Start with an IT Director/Orchestrator, Software Engineer, and QA Engineer.
- [ ] Treat Product Owner and UX/UI as planning/review responsibilities initially; promote them to executable agents when a real workflow benefits.
- [ ] Add bounded multi-step tool execution and explicit per-agent/per-task tool permissions.
- [ ] Add human approval checkpoints for consequential or destructive operations.
- [ ] Separate project knowledge/context from execution history and conversational memory.
- [ ] Evaluate delegation and review using a concrete end-to-end software task before adding more roles.

## Phase 6 — Organization visualization and expansion

- [ ] Add richer monitoring controls only where actual workflows require them.
- [ ] Explore an animated office as an optional visualization of real agent/task states.
- [ ] Keep animation presentation-only; task status and assignments remain backend-owned.
- [ ] Expand beyond IT only after the initial department's task lifecycle, permissions, and monitoring are dependable.

## Phase 7 — Device ecosystem

- [ ] Stabilize a versioned device API contract.
- [ ] Add device identity and authentication.
- [ ] Add device capability discovery.
- [ ] Explore voice, e-ink, sensors, actuators, and home automation as separate modules.
- [ ] Keep the ESP32 Codec client provider-agnostic and compatible with the stable device API.

## Time-sensitive existing feature

The firmware and original project notes describe the Release Countdown screen as temporary, targeting 8:00 AM EST on 2026-12-02. Decide whether to remove it after that date; do not remove it automatically as part of unrelated refactoring.

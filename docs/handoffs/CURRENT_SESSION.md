# SCHUNET — Current Development Handoff

**Last updated:** 2026-11-27  
**Purpose:** Provider-independent starting point. Update this file at the end of each substantive development session.

## Current objective

Lay the foundation for a configurable virtual organization, beginning with IT, while preserving the existing ESP32 client. The first organization workflow must include monitoring from the start. Model selection is task-level and configuration-driven: decompose work, classify each task, then route it to an eligible model profile.

## Repository baseline (from supplied ZIP)

- ESP32 Codec-style client in `schunet-client-2.7/`.
- FastAPI/Ollama bridge in `server/server.py`.
- One ESPN NFL scoreboard tool (`get_nfl_scoreboard`).
- Firmware currently calls synchronous `POST /chat` with an empty JSON body.
- Server also exposes in-memory `/jobs` endpoints, but the ESP32 does not currently use them.
- No hosted provider adapters, provider router, general tool registry, agent framework, persistent task/event history, or desktop/web UI are implemented.
- Supplied ZIP does not include `.git` metadata; branch and working-tree state must be checked in the live repository.

## Accepted project decisions

- Product direction: platform + hardware.
- Intelligence strategy: hybrid local + hosted providers.
- Keep provider and device integrations interchangeable.
- Prefer a small modular Python application before considering separate services.
- Decompose requests into tasks before selecting an execution model; route per task using complexity/capability/risk/privacy/cost/latency/availability.
- Keep provider/model aliases and routing policies configuration-driven; agent-role defaults are preferences, not fixed model assignments.
- Monitoring and structured operational history are part of the first organization workflow.
- Future animated office is a presentation layer over real task/agent state.
- Preserve the ESP32 `/chat` `reply` contract during initial backend refactoring.

See:
- `docs/decisions/001-platform-direction.md`
- `docs/decisions/002-task-routing-and-observability.md`

## Work completed in this planning pass

- Reworked project overview to describe the virtual organization direction and role/task separation.
- Expanded architecture with task-level routing, config-driven model profiles, monitoring events, and the optional office visualization.
- Reordered roadmap so monitoring is built alongside the first organization workflow, not postponed.
- Updated development status to distinguish current implementation from agreed/planned work.
- Added ADR 002 for task-level routing and first-class observability.

## Immediate next steps

1. In the live `feature/corporation` working tree, compare these documentation changes with current files and inspect `git status`/`git diff`; the supplied ZIP cannot verify branch state.
2. Add a minimal dependency manifest and automated test baseline around the existing FastAPI `/health` and `/chat` contracts.
3. Define the first configuration schema (provider settings, model aliases/profiles, routing policy, limits, feature flags); keep credentials in environment variables/local ignored secrets.
4. Introduce a provider-neutral model request/result contract and move the current Ollama integration behind it without changing `/chat` behavior.
5. Add structured execution events and task/run persistence; decide whether SQLite is sufficient for the first local implementation.
6. Build the smallest useful monitoring UI against real backend records before adding broad agent autonomy.
7. Add task lifecycle/decomposition and deterministic routing with unit tests before wiring in hosted inference.

## Guardrails

- Do not assume the Cloudflare AI Gateway account itself defines the underlying inference provider/model. Verify the tested request path and keep gateway and provider configuration distinct.
- Do not silently fall back to paid hosted inference.
- Do not let agent count imply unconstrained Ollama concurrency on the local Mac; use a bounded queue.
- Do not grant agents unrestricted shell/filesystem/Git or external-side-effect permissions. Add scoped tool allowlists and approval gates.
- Do not build the animated office before the real monitoring state model exists.
- Do not remove the temporary countdown screen as part of unrelated work; its documented target is 2026-12-02.

## Session closeout template

At the end of the next implementation session, update this handoff with:

- Objective and outcome
- Files changed
- Tests run and results
- Decisions made
- Known issues
- Exact next step

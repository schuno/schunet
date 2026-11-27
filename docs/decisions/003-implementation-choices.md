# ADR 003: First-slice implementation choices

- **Status:** Accepted (decision 6, the API-shape part of decision 7, and the test expectations in Consequences are amended by ADR 004)
- **Date:** 2026-11-27

## Context

ADR 001 and 002 fixed direction (platform + hardware, hybrid intelligence, task-level routing, first-class monitoring) but deliberately left implementation details open: configuration format, persistence technology, monitoring UI technology, code location, and how resource use is weighed. The model host is a 2020 13-inch Intel MacBook Pro running Ollama on CPU, so every unit of CPU spent by SCHUNET itself is CPU taken from local inference.

## Decisions

1. **Backend package location and name.** New backend code lives in a Python package at `server/ai/` (imports look like `from ai.config import ...`, with `server/` as the working directory/import root). `server/server.py` remains the FastAPI entry point (`uvicorn server:app`) and becomes progressively thinner. The package is a modular monolith: sub-modules, not services.
2. **Configuration format: TOML**, parsed with stdlib `tomllib` (Python 3.11+; the development machine reports 3.12.3). A `tomli` fallback is declared in `requirements.txt` for older interpreters. Validation uses Pydantic models (already a FastAPI dependency). Secrets are never stored in the TOML file: config fields name an environment variable (for example `api_key_env = "..."`), and a local ignored file (`config.toml`) overrides the committed `config.example.toml`.
3. **Operational persistence: SQLite** through the stdlib `sqlite3` module (no ORM), WAL mode, bounded retention. It covers task/run/event history only, not conversational memory. Chosen because it adds no service and no dependency, and its idle CPU cost is effectively zero.
4. **Monitoring UI: static, no-build web page** (plain HTML, CSS, and a small JavaScript file) served by FastAPI. No Node toolchain, no frontend framework to start. Revisit only if a concrete requirement cannot be met.
5. **CPU-first rule.** SCHUNET's own overhead must stay small so local inference gets the CPU. Concretely: no busy loops or fast timers in the server; no per-token logging or per-token event writes; bounded inference concurrency (default 1, configurable); bounded queues and retention; the UI polls with a cursor at a modest interval and pauses while hidden; no heavyweight dependencies without a concrete need. Optimizations to the *inference* flow itself (for example the two-pass tool flow's thinking budget) are out of scope for the refactor and are recorded as candidates in `DEVELOPMENT_STATUS.md`.
6. **Behavior-preserving refactor first (amended by ADR 004: the API shape is no longer constrained; prompts and inference parameters are preserved by default only to keep diffs reviewable).** Prompts, `think` flags, token limits, and the `/chat` response shape are preserved exactly through Steps 1–4 of `docs/IMPLEMENTATION_PLAN.md`. Debug `print` calls are converted to `logging` (a deliberate, non-contract change).
7. **Authentication scope.** Auth arrives with the monitoring/control API. It protects `/api/*` monitoring and control routes only. `/chat`, `/health`, and `/jobs` stay unauthenticated by default because the ESP32 firmware sends no credentials (amended by ADR 004: whether they join the protected set is decided in Step 6a); device identity and auth are Phase 7.
8. **Delivery mechanism.** Changes are exchanged as zip files; the maintainer applies them and reviews the diff. Each step delivers a delta zip containing only added/changed files at repository-relative paths, plus an explicit list of deleted files.

## Consequences

- ADR 002's open items "configuration format" and "persistence technology" are now resolved for the first implementation; exact schema, aliases, and tier thresholds remain open.
- Verification is manual (smoke checks) per ADR 004; no automated test suite is built yet.
- Adding a new dependency requires a stated reason in the session summary.

## Still open

- Exact config schema and routing-tier thresholds (Step 2 proposes; maintainer reviews).
- Event vocabulary details and retention numbers (Step 5; recorded in a follow-up ADR).
- Token handling UX for the monitoring UI (Step 6a).
- Hosted provider/model behind Cloudflare AI Gateway and its tested request shape (needed before Step 8).

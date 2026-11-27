# Step 5 — Structured events + SQLite persistence

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-06a.md`


**Goal:** Every model execution and tool call emits a structured event that is persisted, with unknown telemetry kept distinct from zero.

**Context to include:**
- `server/ai/` (all), `server/server.py`, `server/tests/` (names + the provider/queue tests)
- `docs/ARCHITECTURE.md` §4.6 and §6, ADR 002 ("Monitoring minimum"), ADR 003

**Deliverables:**
- `server/ai/events.py`: event types and a small v1 vocabulary (request received, model execution started/finished/failed, tool invoked/finished/failed; queue wait recorded). Task-level events (decomposition, assignment, review) are defined later in Step 7; keep the vocabulary extensible.
- `server/ai/store.py`: stdlib `sqlite3`, WAL, `synchronous=NORMAL`, schema creation/versioning, `runs` and `events` tables (runs are a minimal precursor to tasks), cheap indexes, retention by age/row count from config, redaction rules (no secrets; prompt/response previews truncated to a configurable length, default modest).
- A non-blocking way to write events (no busy loops, no per-token writes): record one event per lifecycle transition. Explain in the summary how DB writes avoid blocking the event loop.
- Runtime/provider code records provider, model, latency, queue wait, token counts when returned (else NULL/unknown), tool name and outcome, error text (sanitized).
- Config additions: database path, retention, preview length. Database file and directory are git-ignored.
- Tests use a temporary DB; cover persistence across a simulated restart, unknown-vs-zero usage, retention pruning, and redaction.

**Acceptance:** all earlier tests pass; a `/chat` request produces the expected events in SQLite; idle server CPU unchanged (no timers added).

**Docs to update:** new `docs/decisions/004-event-model.md` (vocabulary, schema version, retention, redaction), `ARCHITECTURE.md` §4.6 (implemented subset vs planned), `DEVELOPMENT_STATUS.md`, `ROADMAP.md` Phase 2 boxes (events, persistence, unknown telemetry), progress table, `CURRENT_SESSION.md`.

**Suggested prompt:**

```text
Execute docs/plan/step-05.md (structured events + SQLite persistence).
Read: all of server/ai/, server/server.py, the provider/queue tests in server/tests/,
docs/ARCHITECTURE.md sections 4.6 and 6, docs/decisions/002 and 003.

Add server/ai/events.py and server/ai/store.py (stdlib sqlite3, WAL, retention, redaction),
emit one event per lifecycle transition from the runtime/provider/tool paths, and persist
them. Unknown usage/cost must be stored as NULL, never 0. No timers, no per-token writes.
Explain how writes avoid blocking the event loop. Add config fields (db path, retention,
preview length) and tests using a temp DB. Write docs/decisions/004-event-model.md.
```

**New context after:** yes.

# Step 4 — Tool registry, ESPN extraction, runtime module (Checkpoint A)

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-05.md`


**Goal:** Tools become registered capabilities with a permission boundary; the run flow moves out of `server.py` into a runtime module; `server.py` becomes thin routing.

**Context to include:**
- `server/server.py` (now thin-ish), `server/ai/` (all), `server/tests/` (all)
- `docs/ARCHITECTURE.md` §4.5 and §6; ADR 003

**Deliverables:**
- `server/ai/tools/registry.py`: a `Tool` definition (name, description, JSON-schema parameters, async handler, `requires_approval` flag), registration, and dispatch that **validates model-supplied tool names and arguments**, enforces a per-call allowlist, and returns a structured error instead of raising on unknown or disallowed tools.
- `server/ai/tools/espn.py`: the ESPN scoreboard handler moved verbatim in behavior (shares the app's `httpx` client; no module-level client).
- `server/ai/runtime.py`: the two-pass flow (tool-selection pass, then formatting pass) using provider + registry + queue. Same prompts and parameters as before.
- A provider-neutral tool schema that the Ollama adapter translates.
- Tests: unknown tool name, bad arguments, disallowed tool, ESPN success/non-JSON failure (mocked), full `/chat` tool flow unchanged.

**Acceptance:** all earlier tests pass; `server.py` contains routes, lifespan, and wiring only.

**Docs to update:** `ARCHITECTURE.md` (§2 current components, §4.5 implemented in minimal form), `DEVELOPMENT_STATUS.md` (tool registry implemented; permission policy is minimal allowlist only; approvals still planned), `ROADMAP.md` Phase 1 (all boxes should now be done or explicitly deferred), progress table, `CURRENT_SESSION.md`.

**Suggested prompt:**

```text
Execute docs/plan/step-04.md (tool registry, ESPN extraction, runtime module).
Read: server/server.py, all of server/ai/, all of server/tests/,
docs/ARCHITECTURE.md sections 4.5 and 6, docs/decisions/003.

Extract ESPN into server/ai/tools/espn.py behind a small registry in
server/ai/tools/registry.py (validated names/args, per-call allowlist, structured errors,
requires_approval flag stored but approvals not implemented yet). Move the two-pass flow
into server/ai/runtime.py. Behavior of /chat must remain identical. server.py should end
up as routes + lifespan + wiring. Add the tests listed in Step 4 and keep all earlier
tests green.
```

**Checkpoint A (you, before the next context):** apply the zip, run `pytest`, start the server (`uvicorn server:app --host 0.0.0.0 --port 8000`), `curl` `/health` and `/chat`, then use the real ESP32 once. If anything differs from before, fix it in this context. Then roll the repo zip forward and **open a new context**.

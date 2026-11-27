# Step 6b — Static monitoring UI (Checkpoint B)

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** Step 7 outline in `docs/IMPLEMENTATION_PLAN.md` (turn it into `docs/plan/step-07.md` during the docs consolidation pass first)


**Goal:** The smallest useful monitoring page, reading only from the `/api/*` endpoints.

**Context to include:**
- `server/server.py` (static mounting only), the `/api/*` route module and `docs/SMOKE_CHECKS.md` (for shapes)
- `docs/PROJECT_OVERVIEW.md` "Monitoring and office visualization", ADR 002 ("Monitoring minimum"), ADR 003 (decisions 4 and 5) and ADR 004

**Deliverables:**
- `server/ui/index.html`, `app.js`, `style.css` served by FastAPI `StaticFiles`; no framework, no build step, small bundle.
- Views: live status (busy/idle, queue depth), recent runs list, run detail with an event timeline (provider/model, latency, queue wait, usage shown as "unknown" when null, errors), failures filter. Agent roster and task dependency views show an honest "not configured yet" state until Steps 7 and 9.
- Token entry stored in `sessionStorage`; sent as a bearer header.
- CPU rules: cursor polling about every 2–3 seconds only while `document.visibilityState` is visible, exponential backoff on errors, no animation loops, no per-event DOM rebuilds of the whole list.
- Add a short manual UI checklist to `docs/SMOKE_CHECKS.md` (automated browser tests are out of scope).

**Docs to update:** `DEVELOPMENT_STATUS.md` (monitoring UI: minimal version implemented, list what it does not show), `ROADMAP.md` Phase 2 boxes, `PROJECT_OVERVIEW.md` (move monitoring from "planned" to "partially implemented"), `ARCHITECTURE.md` (UI served at `/ui`), `docs/SMOKE_CHECKS.md`, progress table, `CURRENT_SESSION.md`.

**Suggested prompt:**

```text
Execute docs/plan/step-06b.md (static monitoring UI).
Read: server/server.py, the /api/* module and docs/SMOKE_CHECKS.md (for response shapes),
the "Monitoring and office visualization" section of docs/PROJECT_OVERVIEW.md,
docs/decisions/002 (Monitoring minimum), 003 and 004.

Build server/ui/{index.html,app.js,style.css}, served by FastAPI at /ui, using only
/api/*. Plain HTML/CSS/JS, no framework or build. Show live status, recent runs, run
detail timeline, and errors; show null usage/cost as "unknown"; show honest empty states
for the agent roster and task dependencies. Poll with a cursor every ~2-3s only while the
tab is visible, with backoff on errors. Token stored in sessionStorage. Keep it small and
CPU-light. Add a short manual verification checklist to docs/SMOKE_CHECKS.md.
```

**Checkpoint B (you):** run the `docs/SMOKE_CHECKS.md` sections, send a few `/chat` requests (curl, and the ESP32 if its firmware matches the current API), watch them appear in the UI, restart the server, and confirm history persists. Then **open a new context** and, before Step 7, do a **docs consolidation pass**: have the next session read all docs, reconcile `PROJECT_OVERVIEW.md` / `ARCHITECTURE.md` / `DEVELOPMENT_STATUS.md` with the code, and turn the Step 7–9 outlines in `docs/IMPLEMENTATION_PLAN.md` into `docs/plan/step-07.md` (and so on) using what was learned. Then archive per `docs/WORKFLOW.md` when the milestone closes.

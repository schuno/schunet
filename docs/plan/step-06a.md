# Step 6a — Monitoring read API + auth

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-06b.md`


**Goal:** A read-only JSON API over runs/events and live backend state, protected by a token.

**Context to include:**
- `server/ai/store.py`, `server/ai/events.py`, `server/ai/config.py`, `server/server.py`, `docs/SMOKE_CHECKS.md`
- `docs/decisions/005-event-model.md`, ADR 003 (decision 7), ADR 004 (decision 5), `docs/ARCHITECTURE.md` §6

**Deliverables:**
- `/api/*` routes: recent runs, run detail with ordered events, events after a cursor (`after_id`) for cheap incremental polling, and a live status endpoint (queue depth, busy/idle, configured providers/models with enabled flags; never secrets).
- Bearer-token auth for `/api/*` only, token from an environment variable named in config. If the token is not configured, `/api/*` refuses with a clear message rather than being open. `/chat`, `/health`, `/jobs`: propose in your pre-coding summary whether they join the protected set (ADR 004, decision 5); if protected, note that the ESP32 firmware would need a token (a separate firmware task). Constant-time token comparison; the token never appears in logs or events.
- Pagination limits and a hard cap on response size. `ETag`/cursor behavior kept simple.
- Update `docs/SMOKE_CHECKS.md` with `curl` checks: no token, wrong token, right token, cursor behavior (`after_id`), page-size caps, and that no secret appears in any response.
- **Confirm with you in the pre-coding summary:** the exact token UX (header only; the UI will keep the token in `sessionStorage`).

**Docs to update:** `ARCHITECTURE.md` (API table gains `/api/*`; security section: what is and is not protected), `DEVELOPMENT_STATUS.md` (auth partially implemented: monitoring API only), `ROADMAP.md`, `docs/SMOKE_CHECKS.md`, progress table, `CURRENT_SESSION.md`.

**Suggested prompt:**

```text
Execute docs/plan/step-06a.md (monitoring read API + auth).
Read: server/ai/store.py, server/ai/events.py, server/ai/config.py, server/server.py,
docs/SMOKE_CHECKS.md, docs/decisions/003, 004 and 005, docs/ARCHITECTURE.md section 6.

Add read-only /api/* endpoints (runs, run detail, events after a cursor, live status) with
bearer-token auth on /api/*. Propose whether /chat, /health and /jobs should also be
protected (ADR 004 decision 5). If the token isn't configured, /api/* must refuse with a clear message. Cap page sizes,
never expose secrets, and keep polling cheap (cursor-based). Tell me your token-handling
proposal before coding. Add smoke checks (no automated tests; ADR 004) and update the
API table in docs/ARCHITECTURE.md.
```

**New context after:** yes.

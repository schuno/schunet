# Step 1 — Dependency manifest + characterization tests

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-02.md`


**Goal:** Pin down today's `/health`, `/chat`, and `/jobs` behavior with automated tests *before* refactoring. No change to `server.py` behavior (ideally no change to `server.py` at all).

**Context to include (read these):**
- `server/server.py`, `server/README.md`, `.gitignore`
- `docs/DEVELOPMENT_STATUS.md` ("Verified source facts" section), `docs/ROADMAP.md` (Phase 0), ADR 003

**Do not include:** firmware, designs, util, other ADRs.

**Deliverables:**
- `server/requirements.txt` (fastapi, uvicorn, httpx; `tomli` with marker `python_version < "3.11"`) and `server/requirements-dev.txt` (pytest and nothing else unless justified).
- `server/pytest.ini` (set `pythonpath = .`), `server/tests/` with characterization tests.
- Tests mock Ollama and ESPN by replacing the module-level `client` with an `httpx.AsyncClient(transport=httpx.MockTransport(...))`. Cover: `/health` shape; `/chat` with empty body uses `DEFAULT_PROMPT`; `/chat` with a prompt; reply shape `{"reply": str}`; `<think>` stripping; the tool path (first call returns `tool_calls`, second returns content, ESPN mocked); Ollama HTTP error becomes a 502; `/jobs` create then poll to `done`; unknown job returns 404.
- `.gitignore` additions: `.pytest_cache`, `*.db`, `*.db-wal`, `*.db-shm`, `config.toml`, `.env`, `data/`.
- Expanded `server/README.md`: venv, install, run, test commands.

**Acceptance:** `cd server && pip install -r requirements.txt -r requirements-dev.txt && pytest` passes with no network. Any behavior the tests reveal that looks like a bug is *recorded*, not fixed.

**Docs to update:** `DEVELOPMENT_STATUS.md` (test/dependency baseline implemented; record any quirks found), `ROADMAP.md` Phase 0 checkboxes, `IMPLEMENTATION_PLAN.md` progress table, `CURRENT_SESSION.md`.

**Suggested prompt:**

```text
Execute docs/plan/step-01.md (dependency manifest + characterization tests).
Read: server/server.py, server/README.md, .gitignore, and the "Verified source facts"
section of docs/DEVELOPMENT_STATUS.md, Phase 0 of docs/ROADMAP.md, and
docs/decisions/003-implementation-choices.md.

Goal: add requirements files, pytest config, and tests that pin the CURRENT behavior of
/health, /chat and /jobs (including the two-pass ESPN tool flow, <think> stripping, the
502 on upstream error, and the 404 for unknown jobs) using an injected httpx.MockTransport
in place of server.client. Do not change server.py behavior; if you find something that
looks like a bug, document it in DEVELOPMENT_STATUS.md instead of fixing it. Update
.gitignore and server/README.md as listed in the plan. Use only pytest as a dev dependency.
```

**New context after:** yes.

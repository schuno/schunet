# Step 1 — Dependency manifest + smoke-check baseline

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-02.md`


**Goal:** Make the server reproducibly installable and record how the current prototype behaves as manual smoke checks, before refactoring. There is no automated test suite in this milestone (ADR 004). This step is small; it is fine to run it in the same chat as Step 2 by naming both step files in your prompt.

**Context to include (read these):**
- `server/server.py`, `server/README.md`, `.gitignore`
- `docs/SMOKE_CHECKS.md` (already has a baseline section to verify against the code), `docs/DEVELOPMENT_STATUS.md` ("Verified source facts" section), `docs/ROADMAP.md` (Phase 0), ADR 003 and 004

**Do not include:** firmware, designs, util, other ADRs.

**Deliverables:**
- `server/requirements.txt` (fastapi, uvicorn, httpx; `tomli` with marker `python_version < "3.11"`). No dev requirements file.
- `.gitignore` additions: `*.db`, `*.db-wal`, `*.db-shm`, `config.toml`, `.env`, `data/`.
- Expanded `server/README.md`: venv, install, run, and a pointer to `docs/SMOKE_CHECKS.md`.
- Review the baseline section of `docs/SMOKE_CHECKS.md` against `server/server.py` and correct anything the code contradicts (for example response fields or status codes). Mark any expectation you could only infer from reading code as Inferred.
- No change to `server/server.py` unless required; if you find something that looks like a bug or an awkward API detail, record it in `DEVELOPMENT_STATUS.md` instead of fixing it.

**Acceptance (you run these):** fresh venv, `pip install -r requirements.txt`, server starts, and the baseline smoke checks behave as written. Note any surprises in `CURRENT_SESSION.md`.

**Docs to update:** `DEVELOPMENT_STATUS.md` (dependency manifest implemented; automated tests deferred per ADR 004; record any quirks found), `ROADMAP.md` Phase 0 checkboxes, `docs/SMOKE_CHECKS.md` if corrected, `IMPLEMENTATION_PLAN.md` progress table, `CURRENT_SESSION.md`.

**Suggested prompt:**

```text
Execute docs/plan/step-01.md (dependency manifest + smoke-check baseline).
Read: server/server.py, server/README.md, .gitignore, docs/SMOKE_CHECKS.md, the
"Verified source facts" section of docs/DEVELOPMENT_STATUS.md, Phase 0 of
docs/ROADMAP.md, and docs/decisions/003 and 004.

Goal: add server/requirements.txt, the .gitignore additions and an expanded
server/README.md as listed in the step file, and check the baseline section of
docs/SMOKE_CHECKS.md against the code (fix anything the code contradicts; label
anything inferred). There are no automated tests in this project phase (ADR 004). Do not
change server.py behavior; record awkward or buggy details in DEVELOPMENT_STATUS.md
instead. You can't run Ollama, so say which smoke checks you did not run.
```

**New context after:** yes (or combine with Step 2 as noted above).

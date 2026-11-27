# SCHUNET — Current Session Handoff

**Milestone:** Foundation for a configurable virtual organization (see `docs/IMPLEMENTATION_PLAN.md`)
**Last updated:** 2026-11-27
**Workflow:** `docs/WORKFLOW.md`. Keep this file to about one page; it holds status and the next action only.

## Status

Planning and documentation are complete. **Step 0 is done** (2026-11-27): the docs update was applied in the live `feature/corporation` tree and reviewed with `git diff`, and a virtual environment was created. Development Python is **3.12.3** (so stdlib `tomllib` is available; the `tomli` fallback is not needed locally). **No application code has changed** since the supplied snapshot. Steps 1–9 are not started.

## Last session

- Objective: relax the plan for the proof of concept.
- Outcome: the HTTP API (`/chat`, `/health`, `/jobs`) is no longer a compatibility constraint, and automated tests are deferred (ADR 004). Step 1 is now "dependency manifest + smoke-check baseline"; every step verifies by startup validation, manual checks in `docs/SMOKE_CHECKS.md`, and diff review. Event model ADR renumbered to 005. Docs only; nothing was run.

## Next step

**Step 1** in a new chat: paste the block below with the repo zip attached. It reads `docs/plan/step-01.md` and the files that step lists. Step 1 is small; you may combine it with Step 2 in one chat by naming both step files.

## Next prompt (paste into a fresh chat)

```text
Continuing work on the SCHUNET repo (zip attached; if you can't open zips, tell me
which files to paste). Work under docs/AI_INSTRUCTIONS.md.

Read, in this order: docs/AI_INSTRUCTIONS.md, docs/handoffs/CURRENT_SESSION.md, the
step file named below, then ONLY the files that step file lists. Don't read other
files unless you need them; if you do, tell me which and why. Docs are a guide and
code is the source of truth, so report any discrepancy you find.

Before coding, give me a short restatement of the outcome, the files affected, the
smallest plan, and any compatibility / CPU / security implications. Proceed unless
the plan conflicts with the ADRs or the step file; if it does, stop and ask.

Deliver changes as a zip containing only added/changed files at repo-relative paths
(plus a list of deleted files), not pasted code. If you can't produce zip files, give
each changed file in full, one code block per file with its path. Keep chat summaries
short.

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

## Open issues and risks

- `README.md` and the roadmap refer to `python3 util/schunet.py` and `util/README.md`; the snapshot contains `util/repo_zip.py` and `util/util.zip` instead. Still unresolved; handle as a separate small cleanup, not as part of Step 1.
- The firmware includes `schunet_logo.h`; the file is `SCHUNET_logo.h` (breaks on case-sensitive filesystems). Separate firmware cleanup.
- The root `PROJECT_OVERVIEW.md` is an intentional pointer to `docs/`, not a duplicate.

## Pointers

- Decisions: `docs/decisions/001`, `002`, `003`, `004`.
- Standing rules and guardrails: `docs/AI_INSTRUCTIONS.md`. Manual checks: `docs/SMOKE_CHECKS.md`.
- Time-sensitive: the Release Countdown screen targets 8:00 AM EST on 2026-12-02; do not remove it as part of unrelated work.

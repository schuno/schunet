# Step 0 — Align docs and confirm the live tree (you, no AI needed)

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-01.md`


**Goal:** Make sure the docs and the real working tree agree before code changes.

Do in your live repo on `feature/corporation`:

1. Paste this update's delta zip over the tree. Run `git status` and `git diff`; read the doc changes.
2. Run `python3 --version` (this snapshot's check reported 3.12.3; TOML via `tomllib` needs 3.11+). Create a venv: `python3 -m venv .venv`.
3. Resolve the known discrepancies (or consciously defer them, and note which in `CURRENT_SESSION.md`):
   - `README.md` and the roadmap refer to `python3 util/schunet.py` and `util/README.md`, but the zip contains `util/repo_zip.py` and `util/util.zip`. Decide which is real.
   - `SCHUNET_logo.h` vs `#include "schunet_logo.h"` in the firmware: fix on a case-sensitive filesystem later, as a separate firmware task.
   - `server/README.md` is only a run command; Step 1 expands it.
4. Commit the doc update on its own.

**New context after:** yes, Step 1 starts clean.

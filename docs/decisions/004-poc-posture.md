# ADR 004: Proof-of-concept posture: flexible API surface, manual verification

- **Status:** Accepted
- **Date:** 2026-11-27
- **Amends:** ADR 003 (decision 6, the API-shape part of decision 7, and the test expectations in its Consequences)

## Context

The `/chat`, `/health`, and `/jobs` endpoints exist only because of the first ESP32 experiment; `/chat` was the only one in use. The virtual-organization work is a proof of concept whose API and internals are expected to change quickly. Locking the current API shape, and building an automated test suite around it before the design has settled, would add cost and slow iteration without protecting anything the maintainer depends on.

## Decisions

1. **The HTTP API is not a stability contract during the PoC.** `/chat`, `/health`, and `/jobs` may be changed, replaced, or removed whenever it simplifies the design. The ESP32 is a test client, not a compatibility constraint. Obligations when changing the API:
   - Call out the change in the pre-coding summary, before implementing it.
   - Record it in `DEVELOPMENT_STATUS.md` and `CURRENT_SESSION.md`, including what the ESP32 firmware would need to change.
   - Do not edit firmware in a backend session; firmware changes are a separate session.
   - Do not touch the temporary countdown screen.
   A stable, versioned device API remains a Phase 7 goal.
2. **Automated tests are deferred.** The plan creates no `server/tests/`, `pytest.ini`, or `requirements-dev.txt`. Verification for each step is:
   - startup validation that fails fast with clear messages (configuration, aliases, provider enablement);
   - the manual smoke checks in `docs/SMOKE_CHECKS.md`, which each step extends;
   - the maintainer reading the diff.
   Assistants must say which checks they could not run and must never claim a check passed unless it was run. Results the maintainer reports are labeled as such.
3. **Everything else in ADR 003 stands:** `server/ai/`, TOML config, SQLite, static UI, CPU-first rule, `/api/*` auth, zip delivery. Design requirements that were previously expressed as tests (validate model-supplied tool names and arguments, never silently route to paid inference, unknown telemetry stays unknown) are still requirements on the code, verified by smoke checks instead.
4. **Prompts and inference parameters are preserved by default** through Steps 1–4 (system prompts, `think` flags, token limits), only so the refactor diff stays easy to review. This is a default, not a constraint; a step may change them if it says so and why. Inference optimization is measured separately (see `DEVELOPMENT_STATUS.md`).
5. **Authentication of `/chat`, `/health`, and `/jobs` is an open question.** The firmware sends no credentials today. Decide in Step 6a whether these routes join the protected set.

## Revisit triggers (not commitments)

Reconsider adding a small automated test set when any of these happens:

- Before enabling hosted inference (Step 8): spend gating and "no silent paid fallback" are pure logic with real cost if wrong.
- When the deterministic router (Step 7) becomes load-bearing for task execution.
- When the same regression has appeared twice.

If tests are reintroduced, record it in a new ADR and update `docs/WORKFLOW.md`.

## Consequences

- Fewer files and less setup per step; faster iteration.
- Regressions are caught by manual checks and diff review, so smoke checks must be kept current: every step that changes behavior updates `docs/SMOKE_CHECKS.md`.
- Refactors in Steps 3–4 carry more risk than they would under a test net; the smoke checks and reviewing the diff are the safeguards.

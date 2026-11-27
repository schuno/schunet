# SCHUNET — Implementation Plan (Milestone Index)

**Milestone:** Foundation for a configurable virtual organization (IT department first), with monitoring from the start.
**Last updated:** 2026-11-27
**Lifespan:** This plan covers one milestone. When the milestone is complete (after Checkpoint B and the Step 7–9 hand-off), move this file and `docs/plan/` to `docs/archive/` and write a new plan for the next milestone.

Related: `docs/WORKFLOW.md` (how to run sessions and pass context between chats; reusable opener/closer prompts), `docs/ROADMAP.md` (phases), `docs/decisions/003-implementation-choices.md` (agreed choices), `docs/handoffs/CURRENT_SESSION.md` (where we are right now).

Each step has its own file under `docs/plan/` with the files to load, deliverables, acceptance criteria, docs to update, and a suggested prompt. A session reads **only** its own step file. Outlines for Steps 7–9 below become step files during the docs consolidation pass after Checkpoint B, so they can be refined with what was learned.

## Progress

Update this table at the end of every step. Statuses: Planned, In progress, Done.

| Step | Title | Status | New context after? |
|---|---|---|---|
| 0 | [Align docs and confirm the live tree (you, no AI needed)](plan/step-00.md) | Planned (you, ~10 min) | Yes |
| 1 | [Dependency manifest + smoke-check baseline](plan/step-01.md) | Planned | Yes |
| 2 | [Validated configuration module](plan/step-02.md) | Planned | Yes |
| 3 | [Provider contract, Ollama adapter, lifespan, bounded queue](plan/step-03.md) | Planned | Yes |
| 4 | [Tool registry, ESPN extraction, runtime module (Checkpoint A)](plan/step-04.md) | Planned | Yes (Checkpoint A) |
| 5 | [Structured events + SQLite persistence](plan/step-05.md) | Planned | Yes |
| 6a | [Monitoring read API + auth](plan/step-06a.md) | Planned | Yes |
| 6b | [Static monitoring UI (Checkpoint B)](plan/step-06b.md) | Planned | Yes (Checkpoint B) |
| 7 | Task lifecycle + deterministic complexity router | Planned (outline below) | Yes |
| 8 | First hosted adapter (Cloudflare Gateway path) | Planned (outline below; needs your input) | Yes |
| 9 | IT department roles + bounded agent execution | Planned (outline below) | Yes |

## Ground rules for every step

Summarized from `docs/AI_INSTRUCTIONS.md` and ADR 003; follow the originals if they disagree.

- The HTTP API (`/chat`, `/health`, `/jobs`) is not a stability contract (ADR 004). Call out breaking changes before implementing them, and record what firmware would need to change. Do not edit firmware or the temporary countdown screen in backend sessions.
- Through Step 4, prompts, `think` flags, and token limits are preserved by default so diffs stay reviewable (not a hard constraint). `print` becomes `logging`.
- Never silently route to paid hosted inference. Hosted is off unless explicitly enabled.
- CPU budget: no busy loops, no per-token logging or events, inference concurrency bounded (default 1), bounded queues and retention. State any CPU-relevant choice in the session summary.
- No new dependency without a stated reason. No automated tests during the PoC (ADR 004): verify with startup validation and `docs/SMOKE_CHECKS.md`, which every behavior-changing step extends.
- Label claims Verified / Inferred / Planned. Do not claim any check passed unless it was run.
- Docs are part of the deliverable; each step file lists the docs it must update.

## Later steps (outlines; refine prompts after Checkpoint B)

### Step 7 — Task lifecycle + deterministic complexity router
Task records (goal, acceptance criteria, dependencies, assignee, complexity tier, required capabilities, risk, privacy constraint, status, attempts, result, review outcome), lifecycle states, a deterministic policy router over the config's tiers/aliases, and the routing decision (selected profile, actual provider/model, policy version, human-readable reason) recorded as events. Routing is checked with synthetic tasks and no provider (smoke checks; ADR 004 says to reconsider automated tests here). Add bounded retries/timeouts/step limits. Tasks and dependencies then become visible in the UI. **Read at start:** `server/ai/` (config, events, store, runtime), ARCHITECTURE §4.2–4.3, ADR 002. **Docs:** new ADR for the task model, ARCHITECTURE §4.2/4.3, STATUS, ROADMAP Phase 3.

### Step 8 — First hosted adapter (Cloudflare Gateway path)
**Needs input from you first:** the actual tested gateway endpoint shape, auth method (by env-var name only), the underlying provider, and the model. Capture those in local `config.toml`/environment, never in the repo or the AI context. Then add the adapter behind the existing provider contract, disabled by default, no implicit paid fallback, usage/cost recorded when returned and unknown otherwise, with failure and availability handling covered by smoke checks (ADR 004 says to reconsider automated tests for spend gating before enabling hosted inference). **Docs:** ADR for the hosted path, ARCHITECTURE §4.4, STATUS, ROADMAP Phase 4.

### Step 9 — IT department roles + bounded agent execution
Organization/department/role/agent configuration (IT Director/Orchestrator, Software Engineer, QA Engineer first; Product Owner and UX/UI as planning/review responsibilities), agent roster visible in the UI, bounded multi-step tool use with per-agent/per-task allowlists, approval checkpoints for consequential operations, and one concrete end-to-end software task as the evaluation. Role defaults remain routing *preferences*; task requirements win. **Docs:** ADR for organization config, ARCHITECTURE §4.1, PROJECT_OVERVIEW, STATUS, ROADMAP Phase 5.

---

## Side tasks (do not mix into a step's context)

- **Release Countdown screen:** documented as temporary, target 8:00 AM EST on 2026-12-02. Decide afterwards whether to remove it. Firmware-only session; the backend plan never touches it.
- **`SCHUNET_logo.h` vs `schunet_logo.h` case mismatch** and the **`util/` packaging script/README mismatch:** small, separate cleanup sessions.
- **Inference-flow optimization** (for example the thinking budget in the tool-selection pass, `keep_alive`, model choice): record candidates in `DEVELOPMENT_STATUS.md`; measure before changing; do it after Checkpoint A so the refactor stays behavior-preserving.

---

## Open decisions tracker

| Decision | Needed by | Status |
|---|---|---|
| Config format, persistence, UI tech, package name | — | Resolved (ADR 003) |
| Exact config schema and tier thresholds | Step 2 | Assistant proposes, you review |
| Event retention numbers, preview length | Step 5 | Open |
| Monitoring token UX | Step 6a | Open (proposal in step) |
| Cloudflare Gateway request shape, underlying provider/model | Step 8 | Open (needs you) |
| Hosted-spend controls beyond enabled/disabled (budgets, per-day caps) | Step 8 | Open |

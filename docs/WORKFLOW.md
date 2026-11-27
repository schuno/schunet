# SCHUNET — AI Session Workflow

**Purpose:** How work is handed between AI chat sessions ("contexts") so progress survives context limits and provider changes.
**Provider-neutral by design.** This workflow uses only plain text prompts, file uploads (or pasted files), and files in this repository. It does not depend on any one assistant's memory, projects, custom instructions, canvas/artifact features, or plugins, so it works the same with any assistant or local model.

## Principle

**The repository is the memory.** An AI chat is disposable; anything worth keeping is written into the repo by the end of the session. A new context should be able to continue from the files alone.

## Document layers (and how long each lives)

| Layer | Files | Changes | Read by a new context |
|---|---|---|---|
| Standing | `AI_INSTRUCTIONS.md`, `ARCHITECTURE.md`, `decisions/`, `ROADMAP.md`, this file | Rarely | `AI_INSTRUCTIONS.md` always; others when relevant |
| Milestone | `IMPLEMENTATION_PLAN.md` (index) and `plan/step-*.md` | Status per step; rewritten per milestone | Only the current step file |
| Session | `handoffs/CURRENT_SESSION.md` | Every session | Always, second |

- **Current state of the code** lives in `DEVELOPMENT_STATUS.md` and, above all, in the code (automated tests are deferred; see ADR 004 and `docs/SMOKE_CHECKS.md`).
- When a milestone finishes, move its plan files to `docs/archive/<milestone>/`. Decisions survive in ADRs, status survives in `DEVELOPMENT_STATUS.md`, and the next milestone gets a fresh plan.
- `CURRENT_SESSION.md` stays about one page. It holds status, the last session's outcome, the next step, a ready-to-paste next prompt, and open issues. It does not repeat decisions (those are in ADRs) or standing rules (those are in `AI_INSTRUCTIONS.md`).

## The loop

1. **Start:** open a fresh chat with any assistant. Attach the current repo zip. Paste the **Next prompt** block from `docs/handoffs/CURRENT_SESSION.md`. (It already contains the session opener and the step prompt.)
2. **Plan check:** the assistant restates the outcome, files, plan, and compatibility/CPU/security implications. Redirect it here if needed.
3. **Build:** the assistant works through the step and delivers a delta zip (changed/new files only).
4. **Close:** when the code is done, paste the **Session closer** (below). The assistant updates the docs, rewrites `CURRENT_SESSION.md` including the next prompt block, and delivers the delta zip with the doc changes.
5. **Apply and verify:** unzip the delta over your working tree, read the diff, run the step's checks from `docs/SMOKE_CHECKS.md`, commit.
6. **Roll forward:** re-zip the repo (without secrets) and go back to step 1 in a **new** chat.

### When to open a new context

Open a new chat when any of these is true:

1. A step is finished and its delta zip delivered (the default boundary).
2. You are about to start a different step, even a small one.
3. There have been two or more rounds of failed-check debugging, or large files have been pasted into chat. Run the closer first, then continue from the handoff.
4. The assistant contradicts earlier decisions or re-asks settled questions.
5. You switch between backend, firmware, or a side task.

Stay in the same chat for review feedback on what was just delivered, small fixes to that step's smoke checks, and questions about the diff.

### Switching providers mid-milestone

Nothing changes: paste the Next prompt block into any assistant. If you switch in the middle of a step (no closer run yet), first ask the old assistant to run the closer, or paste your diff and a short description of what is unfinished into the new chat and ask it to write the closeout notes into `CURRENT_SESSION.md`.

### Fallbacks for assistants with limited features

- **Can't open zip files:** upload (or paste) only the files named in the step file, plus `docs/AI_INSTRUCTIONS.md` and `docs/handoffs/CURRENT_SESSION.md`.
- **Can't produce zip files:** have it return each changed file in full, one code block per file with its repo-relative path, and the updated docs as complete files.
- **Can't run the checks** (usually true: no local model or network): it must say which checks it did not run and must not claim any passed. You run them locally from `docs/SMOKE_CHECKS.md`.

## Keeping token use low

- Tell the assistant exactly which files to read; each step file lists them.
- Ask for changes in the delta zip, not as pasted code; ask for a summary of about 25 lines.
- Don't paste whole source files or docs into chat when the zip carries them.
- Never include `secrets.h`, a local `config.toml`, `.env` files, or tokens in anything sent to an assistant.
- Firmware (`schunet-client-2.7/`) and `designs/` are irrelevant to backend steps; leave them out of the zip when you can.

## Reusable prompts

### Session opener

This text is already embedded at the top of the Next prompt block in `CURRENT_SESSION.md`; it is reproduced here as the canonical copy. If you change it, change both.

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
```

### Session closer

```text
Close out this step per docs/AI_INSTRUCTIONS.md "Before ending a session":
1. Write or update the step's smoke checks in docs/SMOKE_CHECKS.md. Run what you can and
   say exactly which checks you could not run; never claim a check passed unless it ran.
2. Update the docs this step requires (see "Docs to update" in the step file):
   the progress table in docs/IMPLEMENTATION_PLAN.md, DEVELOPMENT_STATUS.md, ROADMAP.md
   checkboxes (only for work actually done), ARCHITECTURE.md if structure changed, and
   an ADR if a decision was made. Do not mark anything implemented that isn't.
3. Rewrite docs/handoffs/CURRENT_SESSION.md (about one page): status, what this session
   did, the exact next step, open issues/risks, and a ready-to-paste "Next prompt" block
   (the session opener from docs/WORKFLOW.md followed by the next step file's prompt).
4. Give me the delta zip (changed/new files only, repo-relative paths) and a list of
   deleted files, then a summary of at most 25 lines: files changed, behavior changed,
   checks run vs not run, decisions, known issues.
```

### Session notes template (structure of `CURRENT_SESSION.md`)

- **Status:** milestone, last completed step, date
- **Last session:** objective, outcome, files changed, checks run and results (and which were not run)
- **Next step:** which step file, which files to read
- **Next prompt:** copy-paste block
- **Open issues and risks**
- **Pointers:** links to ADRs and plan files; no copied content

# SCHUNET

SCHUNET is an experimental hybrid AI and hardware platform. Its current prototype is an ESP32 Codec-style TFT client connected to a Python/FastAPI bridge and a local Ollama model. The intended direction is provider-agnostic orchestration across local and hosted models, tools, agents, and connected devices.

## Documentation

Start with [`docs/PROJECT_OVERVIEW.md`](docs/PROJECT_OVERVIEW.md), then see the [architecture](docs/ARCHITECTURE.md), [development status](docs/DEVELOPMENT_STATUS.md), [roadmap](docs/ROADMAP.md), [AI coding instructions](docs/AI_INSTRUCTIONS.md), plus the [AI session workflow](docs/WORKFLOW.md).

## Working with AI assistants (passing context between chats)

The repository is the memory; each AI chat is disposable. This flow works with any assistant (no provider-specific features). Full details: [`docs/WORKFLOW.md`](docs/WORKFLOW.md).

**One step = one fresh chat.** For each step of the current plan ([`docs/IMPLEMENTATION_PLAN.md`](docs/IMPLEMENTATION_PLAN.md)):

1. **Start a new chat** with any assistant and attach the current repo zip (leave out `secrets.h`, local `config.toml`, `.env`, and tokens).
2. **Paste the "Next prompt" block** from [`docs/handoffs/CURRENT_SESSION.md`](docs/handoffs/CURRENT_SESSION.md). It already includes the instructions and the step prompt.
3. **Check the plan** the assistant restates before it codes, then let it build and deliver a zip of only the changed files.
4. **Close out:** paste the "Session closer" from [`docs/WORKFLOW.md`](docs/WORKFLOW.md). The assistant updates the docs, rewrites `CURRENT_SESSION.md` with the next prompt, and gives you a final zip.
5. **Apply and verify:** unzip over your working tree, read the diff, run `cd server && pytest`, do the step's manual checks, commit.
6. **Re-zip the repo and repeat from step 1 in a new chat.**

Where things live: standing rules in `docs/AI_INSTRUCTIONS.md`, decisions in `docs/decisions/`, the milestone plan in `docs/IMPLEMENTATION_PLAN.md` plus one file per step in `docs/plan/`, and the live handoff in `docs/handoffs/CURRENT_SESSION.md`. Switching providers needs no changes: just paste the Next prompt into the new assistant.

## Generate an AI context package

From the repository root, run:

```bash
python3 util/schunet.py
```

This creates `schunet_ai_context.zip`, a compact package containing project documentation, relevant text/source files, and Git working-tree status. See [`util/README.md`](util/README.md) for details.

# SCHUNET

SCHUNET is an experimental hybrid AI and hardware platform. Its current prototype is an ESP32 Codec-style TFT client connected to a Python/FastAPI bridge and a local Ollama model. The intended direction is provider-agnostic orchestration across local and hosted models, tools, agents, and connected devices.

## Documentation

Start with [`docs/PROJECT_OVERVIEW.md`](docs/PROJECT_OVERVIEW.md), then see the [architecture](docs/ARCHITECTURE.md), [development status](docs/DEVELOPMENT_STATUS.md), [roadmap](docs/ROADMAP.md), and [AI coding instructions](docs/AI_INSTRUCTIONS.md).

## Generate an AI context package

From the repository root, run:

```bash
python3 util/schunet.py
```

This creates `schunet_ai_context.zip`, a compact package containing project documentation, relevant text/source files, and Git working-tree status. See [`util/README.md`](util/README.md) for details.

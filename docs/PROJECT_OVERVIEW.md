# SCHUNET — Project Overview

**Status:** Early-stage, working prototype  
**Last reviewed:** 2026-11-27

## What SCHUNET is

SCHUNET is an experimental AI and hardware platform. Its intended direction is to provide a common environment for experimenting with local and frontier AI models, tools, agents, and connected physical devices.

The selected product direction is **platform + hardware**, with a **hybrid intelligence strategy**: local models and hosted frontier providers should be usable side by side, with the choice depending on the task.

The current implementation is much smaller than that long-term direction. Today it consists of an ESP32 Codec-style display client and a Python/FastAPI bridge that calls a local Ollama model and one ESPN data tool. Cloud model providers, a general model router, a reusable agent framework, and a desktop/web control plane are not implemented yet.

## Current working prototype

### ESP32 Codec client

- ESP32-based client with a 2.7-inch, 240×320 TFT display in portrait orientation.
- Uses `TFT_eSPI` / `TFT_eSprite` for rendering and partial-screen animation.
- Uses Wi-Fi and HTTP to communicate with the server.
- The interface uses a phosphor-green, Metal Gear Solid Codec-inspired visual style.
- Includes a home screen, an AI Agent screen, and a temporary release-countdown screen.
- The microphone-shaped control currently triggers a request; it does not capture audio.
- The firmware posts to the server's synchronous `/chat` endpoint and renders the returned reply with a local typewriter animation.

### Python bridge server

- Located in `server/server.py`.
- Built with FastAPI, `httpx`, and Pydantic.
- Calls Ollama's `/api/chat` endpoint.
- Defaults to `qwen3.5:9b`, configurable with `MODEL`.
- Defaults to `http://127.0.0.1:11434`, configurable with `OLLAMA_URL`.
- Contains one hard-coded tool, `get_nfl_scoreboard`, which retrieves NFL scoreboard data from ESPN.
- Uses a tool-selection pass followed by a formatting pass when the model requests the ESPN tool.
- Exposes `/health`, `/chat`, `/jobs`, and `/jobs/{job_id}`.

## Intended direction

SCHUNET should own the orchestration and client experience, while keeping model providers and device types replaceable.

1. **Provider agnostic:** application features should depend on a shared provider contract rather than directly on Ollama or a specific hosted provider.
2. **Device agnostic:** the ESP32 is the first client, not the only client. Desktop, web, e-ink, voice, and other embedded clients may be added later.
3. **Composable capabilities:** tools and integrations should be registered independently of model-provider code.
4. **Hybrid intelligence:** route work between local and hosted models according to capability, latency, cost, privacy, and available resources.
5. **Understandable infrastructure:** prefer small, explicit components and avoid introducing heavyweight frameworks or services without a demonstrated need.

## Current hardware and development environment

The project overview supplied with the repository identifies the current model host as a 2020 13-inch Intel MacBook Pro running Ollama with `qwen3.5:9b` using CPU inference. This is documented project context, not a runtime-discovered hardware inventory.

The ESP32 firmware expects network settings from a local `secrets.h` file. The repository contains `secrets_placeholder.h` as a template; actual credentials should not be committed or included in AI context packages.

## Repository map

```text
.
├── docs/                         # Canonical project documentation and handoffs
├── schunet-client-2.7/            # ESP32 firmware and display assets
├── server/                        # FastAPI-to-Ollama bridge
├── util/                          # Local developer utilities
├── assets/                         # Project artwork
├── designs/                        # Mechanical/CAD assets
├── PROJECT_OVERVIEW.md             # Compatibility pointer to docs/ version
└── README.md
```

## Scope boundaries

The following are goals or possible future capabilities, not claims about current implementation:

- OpenAI, Anthropic, Gemini, or other hosted provider adapters.
- Automatic model selection or routing.
- General-purpose agent orchestration or multi-agent delegation.
- Persistent conversational memory.
- General-purpose tool/plugin discovery.
- Desktop or web administration UI.
- Voice capture or speech-to-text.
- General home automation or arbitrary hardware control.

When describing SCHUNET, distinguish clearly between **implemented**, **partially implemented**, **planned**, and **speculative** capabilities.

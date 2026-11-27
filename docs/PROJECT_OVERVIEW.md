# SCHUNET — Project Overview

**Status:** Early-stage working prototype; organization platform is planned  
**Last reviewed:** 2026-11-27

## What SCHUNET is

SCHUNET is an experimental AI and hardware platform. Its intended direction is to provide a common environment for experimenting with local and hosted AI models, tools, agents, and connected physical devices.

The selected product direction is **platform + hardware**, with a **hybrid intelligence strategy**. SCHUNET owns orchestration and the client experience while keeping model providers and device clients interchangeable.

The next major product step is a **configurable virtual organization**, beginning with an IT department. Work will be broken into bounded tasks, assigned to role-based agents, and routed to models according to the requirements of each task. Agent roles describe responsibilities and working instructions; they do not permanently dictate which model must run them.

A monitoring/control interface is part of the initial organization foundation, not a later polish phase. A future animated office may visualize agent presence and activity, but it is a presentation layer over real organization/task state—not the source of truth and not a prerequisite for execution.

The current implementation is much smaller than this target. Today it consists of an ESP32 Codec-style display client and a Python/FastAPI bridge that calls a local Ollama model and one ESPN data tool. Cloud model adapters, model routing, reusable agent orchestration, and a desktop/web control plane are not implemented yet.

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

## Intended organization model

The first organization is an IT department. It should begin with a small number of executable roles and grow only after the task lifecycle and monitoring are reliable.

Initial role concepts:

- **IT Director / Orchestrator:** clarifies objectives, decomposes work, assigns tasks, tracks dependencies, and reviews completion.
- **Software Engineer:** implements bounded engineering tasks. Junior/senior levels may be represented as role profiles or task responsibilities, but the actual model is selected by routing policy.
- **QA Engineer:** creates and runs appropriate checks, evaluates results, and reports defects or confidence gaps.
- **Product Owner and UX/UI roles:** can initially be planning/review responsibilities and become independently executable agents when useful.

An agent definition should hold stable role information such as name, department, responsibilities, instructions, allowed tools, and default routing profile. Each task should carry its own requirements and constraints. The runtime combines both to choose an execution model.

## Model selection and configuration

Model selection happens **after task decomposition**, when the system has a concrete unit of work to evaluate. Routing should consider more than the agent's title:

- Task complexity and reasoning depth.
- Required capabilities (for example, code generation, analysis, vision, or tool use).
- Risk and consequence of an incorrect result.
- Context size and output requirements.
- Privacy/data handling constraints.
- Latency, estimated cost, provider availability, and local resource pressure.

Configuration should define provider connections, model aliases/profiles, capability metadata, limits, and routing policies. Agent and task definitions refer to aliases or policy names—not hard-coded vendor model IDs. Role defaults are useful fallbacks, not immutable assignments. A routing decision should be recorded with its reason so it can be inspected in the monitoring UI.

The initial system should support explicit model selection and a small, understandable policy before attempting sophisticated automatic routing. Hosted calls must be opt-in/configured; a missing or invalid hosted configuration must not silently trigger paid inference. Cloudflare AI Gateway is treated as a gateway/observability and routing component in front of configured inference providers, not automatically as the model provider itself.

## Monitoring and office visualization

The first organization milestone includes a usable monitoring UI. At minimum it should show:

- Tasks, dependencies, status, assignee, and recent activity.
- Agent roster and availability/busy state.
- Selected provider/model and routing explanation for each execution.
- Start/end time, duration, token usage when available, estimated cost when available, and errors.
- Tool invocations and approval/waiting states.

The UI should read from structured task and execution events produced by the backend. Persist enough task/event history to make the monitor useful across page refreshes and server restarts. This is operational history, not a commitment to build long-term conversational memory at the same time.

A future animated office can map real agent/task states to visual behaviors (at desk, walking to a meeting, reviewing work, idle). It must remain decorative: animation must never invent status, change assignments, or become necessary to operate the organization.

## Intended platform boundaries

1. **Organization layer:** departments, roles, agent definitions, task decomposition, assignment, dependencies, review, and policy.
2. **Orchestration layer:** task lifecycle, context assembly, execution loop, model routing, approvals, and event emission.
3. **Provider layer:** normalized model interface and adapters for Ollama and configured hosted inference providers.
4. **Capability layer:** tool registry, schemas, permission checks, and execution results.
5. **Observability/control plane:** monitoring UI, task/event history, model usage, and later visualizations.
6. **Client/device layer:** web/desktop interfaces, ESP32, and future devices.
7. **Infrastructure:** configuration, secrets, persistence, logging, metrics, and access control.

Keep these as modules within the existing Python application initially. Do not split them into separate services without a concrete scaling or deployment reason.

## Current hardware and development environment

The project overview supplied with the repository identifies the current model host as a 2020 13-inch Intel MacBook Pro running Ollama with `qwen3.5:9b` using CPU inference. This is documented project context, not a runtime-discovered hardware inventory.

The ESP32 firmware expects network settings from a local `secrets.h` file. The repository contains `secrets_placeholder.h` as a template; actual credentials should not be committed or included in AI context packages.

## Scope boundaries

The following are goals or possible future capabilities, not claims about current implementation:

- Config-driven task-complexity model routing.
- Hosted provider adapters and Cloudflare AI Gateway integration in the runtime.
- General-purpose agent orchestration and a virtual IT department.
- Persistent task/execution history and operational monitoring UI.
- Animated office/agent visualization.
- Persistent conversational memory.
- Desktop-native control plane, voice capture, speech-to-text, and general home automation.

When describing SCHUNET, distinguish clearly between **implemented**, **partially implemented**, **planned**, and **speculative** capabilities.

# SCHUNET — Instructions for AI Coding Assistants

You are contributing to SCHUNET, an early-stage hybrid AI experimentation and hardware platform. Read this file and the relevant documents in `docs/` before making changes.

## Required context

Start with:

1. `docs/PROJECT_OVERVIEW.md`
2. `docs/ARCHITECTURE.md`
3. `docs/DEVELOPMENT_STATUS.md`
4. `docs/ROADMAP.md`
5. `docs/handoffs/CURRENT_SESSION.md`, if present and relevant

Then inspect the actual source files involved in the task. Documentation is a guide, not proof that behavior still exists.

## Project direction

- Product direction: platform + hardware.
- Intelligence strategy: hybrid local and hosted providers.
- SCHUNET should own orchestration and the client experience, not bind the system to one model vendor or device.
- The ESP32 Codec terminal is the first working client and should remain functional during backend evolution.
- The virtual organization is built from task-oriented agents; model selection happens per decomposed task, not as a permanent model assignment per role.
- Provider/model aliases and routing policies are configuration-driven. Never silently route to paid hosted inference.
- Structured task/execution events and a useful monitoring UI are foundational to the first organization workflow. A future animated office is presentation-only and must reflect real backend state.

## Engineering principles

- Prefer small, explicit abstractions over speculative frameworks.
- Keep provider-specific code inside provider adapters.
- Keep tools/capabilities separate from model providers.
- Keep agent role definitions separate from model profiles; route each task using complexity, capability, risk, privacy, cost, latency, and availability.
- Record routing decisions and reasons so they can be inspected and tested.
- Treat operational task history as distinct from conversational memory. Build monitoring on structured events, not inferred UI state.
- Keep device API contracts independent of provider-specific response formats.
- Do not introduce heavyweight services or dependencies without a concrete need.
- Preserve backward compatibility where practical; call out breaking changes before implementing them.
- Do not invent functionality or mark planned work as implemented.
- Do not silently change secrets, network exposure, tool permissions, or device behavior.

## Before changing code

1. Restate the requested outcome and identify affected files/components.
2. Inspect the current implementation and relevant tests.
3. Explain the smallest viable implementation plan.
4. Identify compatibility, security, and resource implications.

## While changing code

- Keep changes scoped to the requested task.
- Follow existing style unless there is a reason to improve it.
- Add or update tests for behavior changes.
- Do not commit credentials, `.env` files, private keys, Wi-Fi passwords, or API tokens.
- Do not assume a tool call from a model is safe; validate tool names and arguments and apply explicit permissions.

## Before ending a session

1. Run relevant tests or explain why they could not be run.
2. Summarize files changed and behavior changed.
3. Record unresolved issues and risks.
4. Update `docs/DEVELOPMENT_STATUS.md` if implementation status materially changed.
5. Update `docs/ARCHITECTURE.md` or add a decision record if architecture materially changed.
6. Update `docs/handoffs/CURRENT_SESSION.md` with the next useful step.
7. Do not claim tests passed unless they were actually run.

## Handling uncertainty

Clearly label statements as one of:

- **Verified:** directly confirmed in current source or by a test.
- **Inferred:** likely based on code structure, but not directly verified at runtime.
- **Planned:** an agreed or proposed future capability.

If repository code and documentation disagree, report the discrepancy and use current code as the implementation source of truth unless the task explicitly says otherwise.

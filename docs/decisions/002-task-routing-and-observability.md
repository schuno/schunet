# ADR 002: Task-level model routing and first-class monitoring

- **Status:** Accepted direction; implementation details remain open
- **Date:** 2026-11-27

## Context

SCHUNET is expanding from a single-device Ollama proof of concept toward a configurable virtual organization. Different work units have different reasoning, capability, risk, privacy, latency, and cost requirements. A fixed model per agent role would be too rigid: even one software engineer role may receive both routine tasks and difficult architecture/debugging work.

The organization also needs operational visibility while it is being built. Without task and execution events from the beginning, a monitoring UI would be forced to infer activity from transient state or provider logs. A future animated office is desirable as a playful visualization, but it should not become a second source of truth.

## Decisions

1. **Decompose before execution routing.** Broad requests should be broken into structured tasks before choosing the execution model for each task.
2. **Route per task, not solely per agent.** Task complexity, required capabilities, risk, context size, privacy constraints, latency, cost, provider availability, and local resource pressure inform model selection.
3. **Keep routing configuration-driven.** Provider connections, model aliases/profiles, capability metadata, limits, and routing policies live in configuration. Agent code refers to aliases/policy names rather than vendor model IDs.
4. **Use role defaults as preferences/fallbacks.** An agent may have a default routing profile, but task requirements take precedence. “Junior” and “senior” are useful organizational concepts, not permanent model bindings.
5. **Start with deterministic routing.** Use a small, understandable set of complexity tiers and auditable rules. Consider more adaptive/scored routing only after usage and evaluation data exist.
6. **No implicit paid fallback.** Hosted inference is explicitly configured/enabled. A missing local model or provider failure must not silently cause paid inference.
7. **Monitoring is foundational.** Task lifecycle and model execution emit structured events. Persist enough operational history for a monitoring UI from the first organization workflow.
8. **Office animation is presentation-only.** A future animated office consumes real agent/task state and cannot own or modify orchestration state.
9. **Keep the architecture modular-monolithic initially.** These capabilities are modules within the current Python application until a concrete reason justifies service separation.

## Initial routing dimensions

- Complexity/reasoning demand.
- Required capabilities and context/output limits.
- Risk and consequence of failure.
- Privacy/data handling eligibility.
- Cost, latency, provider availability, and local resource pressure.

A simple initial policy may use tiers such as `routine`, `standard`, `demanding`, and `critical`. Exact thresholds, model assignments, and model identifiers are configuration decisions to be made after provider capabilities and local constraints are measured.

## Monitoring minimum

The first useful monitoring view should expose task status, dependencies, agent assignment/availability, routing decision and reason, provider/model, execution timing, token usage/cost when available, tool activity, approvals, and errors. Unknown telemetry must remain explicitly unknown.

## Consequences

- A provider-neutral model contract and validated configuration are prerequisites for robust routing.
- Task and execution records become core domain concepts rather than incidental log messages.
- Monitoring needs structured events and persistence, but does not require general conversational memory in the same milestone.
- The initial UI can be a practical web control surface; the exact frontend technology remains open.
- Routing policies require tests independent of live model providers.
- Provider/gateway usage and SCHUNET task observability are separate concerns. Cloudflare AI Gateway may provide gateway-level visibility, while SCHUNET records organization-level task and agent state.

## Not decided by this ADR

- Exact configuration format and schema.
- Exact provider/model aliases and complexity thresholds.
- Which hosted inference provider/model is used behind Cloudflare AI Gateway.
- Persistence technology (SQLite is a candidate for the first local implementation).
- Monitoring UI framework and visual design.
- Whether the animated office is 2D, 3D, or a game-like interface.

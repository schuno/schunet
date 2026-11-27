# ADR 001: SCHUNET product direction and intelligence strategy

- **Status:** Accepted
- **Date:** 2026-11-27

## Context

SCHUNET began as an ESP32 Codec-style display connected to a local Ollama model through a Python bridge. The intended scope has expanded toward experimentation with local models, frontier providers, software tools, agents, and physical hardware.

## Decision

SCHUNET will be developed as a **platform + hardware** project, using a **hybrid intelligence strategy**.

- Platform: model/provider integration, orchestration, agents, tools, experimentation, and observability.
- Hardware: ESP32 and future connected devices are first-class clients and capability endpoints.
- Hybrid intelligence: local models and hosted frontier models can both be used, selected according to task needs and explicit policy.

SCHUNET should own the orchestration and experience while keeping model providers and device clients interchangeable.

## Consequences

- Avoid coupling application features directly to Ollama or a single hosted provider.
- Establish a provider-neutral internal contract before adding multiple providers.
- Keep device communication independent of model vendor.
- Keep tools separate from providers.
- Prefer a modular monolith initially; do not prematurely split the application into services.
- Treat the current ESP32 client as the first supported device, not the definition of the entire platform.

## Not decided by this ADR

- Which hosted provider is added first.
- The exact automatic routing policy.
- Which agent framework, if any, will be used.
- The persistence technology for sessions or memory.
- The design of a future desktop/web interface.

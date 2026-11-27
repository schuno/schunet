# Step 2 — Validated configuration module

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-03.md`


**Goal:** A typed, validated configuration layer that can describe providers, model aliases/profiles, routing tiers, limits, and feature flags, without wiring it into request handling yet (wiring happens in Step 3).

**Context to include:**
- `docs/ARCHITECTURE.md` sections 4.3, 5, and 6; ADR 003 (decision 2 especially); ADR 002 ("Initial routing dimensions")
- `server/server.py` only for the env-var defaults (`OLLAMA_URL`, `MODEL`)
- `.gitignore` and `docs/SMOKE_CHECKS.md`

**Deliverables:**
- `server/ai/__init__.py`, `server/ai/config.py` (Pydantic models + loader), `server/config.example.toml`.
- Load order: built-in defaults that reproduce today's behavior, then `config.toml` (path overridable by an environment variable, with the name documented), then `OLLAMA_URL` / `MODEL` env overrides as a convenience (may be dropped if they complicate the design).
- Model profiles carry provider, model id, capabilities, context/output limits, a small capability tier, local/hosted flag, and optional cost metadata (unknown allowed, never defaulted to zero).
- Routing: tiers `routine/standard/demanding/critical` map to model aliases; `allow_paid_fallback = false` default; hosted provider `enabled = false` default.
- Limits: `max_concurrent_inference` (default 1), request timeout, queue bound, retention settings placeholder.
- Startup validation: every alias referenced by routing exists; every model's provider exists; routing to a model whose provider is disabled is a clear startup error (no silent fallback); secrets referenced by env-var *name*, never stored; error messages never print secret values.

**Acceptance (you run these; the assistant writes them into `docs/SMOKE_CHECKS.md` as numbered checks):** loading with no `config.toml` gives defaults equal to today's behavior; each invalid case below fails fast with a clear message that never prints secret values: unknown alias in routing, model referencing a missing provider, routing to a model whose provider is disabled, enabled hosted provider with its env var unset. The module does not read the network or the filesystem at import time.

Because there is no test suite, give each invalid case a one-line way to reproduce it (for example "edit this key in `config.toml`, start the server, expect this message").

**Docs to update:** `ARCHITECTURE.md` (§5 now describes the real schema; note it is implemented but unwired), `DEVELOPMENT_STATUS.md`, `ROADMAP.md` Phase 1 boxes (config + secrets validation), `docs/SMOKE_CHECKS.md`, progress table, `CURRENT_SESSION.md`. Add `server/config.example.toml` to the docs' file lists where relevant.

**Suggested prompt:**

```text
Execute docs/plan/step-02.md (validated configuration module).
Read: docs/ARCHITECTURE.md sections 4.3, 5, 6; docs/decisions/002, 003 and 004; the
env-var defaults at the top of server/server.py; and .gitignore and docs/SMOKE_CHECKS.md.

Create server/ai/config.py (Pydantic models + loader), server/config.example.toml and
smoke checks (no automated tests; ADR 004), per Step 2's deliverables. TOML via stdlib
tomllib. Defaults should reproduce current behavior unless you propose and justify a
better default (OLLAMA_URL, MODEL env vars may stay as a convenience). Validate aliases and
provider enablement at load time; hosted disabled by default; no paid fallback; secrets
only by env-var name. Do NOT wire it into server.py yet. Keep it small and CPU-cheap
(no work at import time). Propose the schema in your pre-coding summary so I can object.
```

**New context after:** yes.

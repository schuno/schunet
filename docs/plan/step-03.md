# Step 3 — Provider contract, Ollama adapter, lifespan, bounded queue

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-04.md`


**Goal:** Move the Ollama call behind a provider-neutral interface and replace the bare `gpu_lock` with a configurable bounded inference queue, keeping the ESP32's default chat flow working where practical (the HTTP API shape may change; see ADR 004).

**Context to include:**
- `server/server.py`, `server/ai/config.py`, `docs/SMOKE_CHECKS.md`
- `docs/ARCHITECTURE.md` §4.4 and §6; ADR 003 and 004

**Deliverables:**
- `server/ai/providers/base.py`: minimal `ModelRequest` / `ModelResult` (text, tool calls, provider, model, latency; usage tokens as `None` = unknown, never 0) and a provider protocol. Keep Ollama-specific options (`think`, `keep_alive`, `num_predict`, `temperature`) out of the neutral contract's required fields; carry them as clearly named optional/extra parameters.
- `server/ai/providers/ollama.py`: translation to/from Ollama `/api/chat`, including tool schema translation.
- An inference queue (semaphore sized by `max_concurrent_inference`, with a bounded wait/queue depth, and a clear error when full). No polling loops.
- FastAPI lifespan that creates and closes the `httpx.AsyncClient`; `server.py` loads config and builds the provider through it.
- `print` calls replaced with `logging` (no prompt/secret leakage, no per-token logging).
- Update `docs/SMOKE_CHECKS.md`: adapter works end to end, two concurrent `/chat` requests with `max_concurrent_inference = 1` are served one after another, queue-full returns a clean error, and the server shuts down cleanly (client closed). If you change the HTTP API, list the change and its firmware impact.

**Acceptance (you run these):** the baseline and new smoke checks behave as written; any API change was called out beforehand and recorded.

**Docs to update:** `ARCHITECTURE.md` (§2.2 and §2.3 current-state text, §4.4 now implemented for Ollama, §6 lifespan note resolved), `DEVELOPMENT_STATUS.md` (move provider contract and Ollama adapter to implemented; update "Known implementation limitations"), `ROADMAP.md`, `docs/SMOKE_CHECKS.md`, progress table, `CURRENT_SESSION.md`.

**Suggested prompt:**

```text
Execute docs/plan/step-03.md (provider contract, Ollama adapter, lifespan,
bounded inference queue).
Read: server/server.py, server/ai/config.py, docs/SMOKE_CHECKS.md,
docs/ARCHITECTURE.md sections 4.4 and 6, and docs/decisions/003 and 004.

Introduce server/ai/providers/{base,ollama}.py, an inference queue driven by
limits.max_concurrent_inference, and an app lifespan that owns the httpx client.
Wire server.py to config + provider. Keep prompts, think flags and token limits as they are by default;
the HTTP API shape may change, but call any change out before making it. Convert print
to logging. Usage fields must be None when unknown. No automated tests (ADR 004):
add smoke checks for the adapter, queue serialization, queue-full, and shutdown, and
say which you could not run.
```

**New context after:** yes.

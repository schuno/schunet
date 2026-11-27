# Step 3 — Provider contract, Ollama adapter, lifespan, bounded queue

**Part of:** `docs/IMPLEMENTATION_PLAN.md` (milestone: configurable virtual organization foundation). Read this file for this step only.
**Workflow:** start with the session opener and finish with the session closer from `docs/WORKFLOW.md`.
**Next step file:** `docs/plan/step-04.md`


**Goal:** Move the Ollama call behind a provider-neutral interface and replace the bare `gpu_lock` with a configurable bounded inference queue, with `/chat` behavior unchanged.

**Context to include:**
- `server/server.py`, `server/ai/config.py`, `server/tests/` (all)
- `docs/ARCHITECTURE.md` §4.4 and §6; ADR 003

**Deliverables:**
- `server/ai/providers/base.py`: minimal `ModelRequest` / `ModelResult` (text, tool calls, provider, model, latency; usage tokens as `None` = unknown, never 0) and a provider protocol. Keep Ollama-specific options (`think`, `keep_alive`, `num_predict`, `temperature`) out of the neutral contract's required fields; carry them as clearly named optional/extra parameters.
- `server/ai/providers/ollama.py`: translation to/from Ollama `/api/chat`, including tool schema translation.
- An inference queue (semaphore sized by `max_concurrent_inference`, with a bounded wait/queue depth, and a clear error when full). No polling loops.
- FastAPI lifespan that creates and closes the `httpx.AsyncClient`; `server.py` loads config and builds the provider through it.
- `print` calls replaced with `logging` (no prompt/secret leakage, no per-token logging).
- Existing characterization tests still pass unchanged (adapt only the mock injection point if required, and say so). New tests for adapter translation and queue behavior.

**Acceptance:** all Step 1 tests pass; `/chat` reply shape identical; two requests with `max_concurrent_inference = 1` are serialized; queue-full path returns a clean error.

**Docs to update:** `ARCHITECTURE.md` (§2.2 and §2.3 current-state text, §4.4 now implemented for Ollama, §6 lifespan note resolved), `DEVELOPMENT_STATUS.md` (move provider contract and Ollama adapter to implemented; update "Known implementation limitations"), `ROADMAP.md`, progress table, `CURRENT_SESSION.md`.

**Suggested prompt:**

```text
Execute docs/plan/step-03.md (provider contract, Ollama adapter, lifespan,
bounded inference queue).
Read: server/server.py, server/ai/config.py, everything in server/tests/,
docs/ARCHITECTURE.md sections 4.4 and 6, and docs/decisions/003.

Introduce server/ai/providers/{base,ollama}.py, an inference queue driven by
limits.max_concurrent_inference, and an app lifespan that owns the httpx client.
Wire server.py to config + provider. Preserve /chat behavior exactly (same prompts,
think flags, token limits, reply shape); convert print to logging. Usage fields must be
None when unknown. Existing tests must keep passing; tell me if you must change a test
and why. Add tests for adapter translation and queue serialization/full-queue behavior.
```

**New context after:** yes.

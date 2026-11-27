# SCHUNET ESP32 Codec Terminal — Project Documentation

## Overview
**SCHUNET** is an embedded interactive terminal interface running on an **ESP32 microcontroller** paired with a **2.7-inch TFT display** (`240×320` portrait orientation, driven via `TFT_eSPI`). The user interface is styled after the classic *Metal Gear Solid* Codec communication screens, featuring phosphor-green vector/bitmap graphics, CRT scanline overlays, and modular navigation.

The terminal talks to a **local LLM** (`qwen3.5:9b` served by Ollama) running on a laptop on the same Wi-Fi network, through a small Python bridge service (`server.py`). Tapping the microphone button on the AI Agent screen sends a request, shows a bouncing-dots "thinking" animation, executes any external API tools requested by the model, and **typewriter-streams** the reply into the dialogue box while animating the agent's mouth in sync with the speech. This tool-calling loop was successfully validated end-to-end using a live ESPN NFL scoreboard tool (`get_nfl_scoreboard`), laying the groundwork for future multi-tool capabilities.

A second screen, a flying-rocket **release countdown**, was added as a fun, **temporary** novelty for the 8:00 AM EST, Dec 2, 2026 "Assisted Voice Auto Production Release". It is expected to be removed after that date (see the roadmap).

---

## Hardware & Software Stack
* **Microcontroller:** ESP32 (Wi-Fi Station Mode enabled)
* **Display:** 2.7" TFT LCD (`240×320` px, `tft.setRotation(0)`)
* **Graphics Engine:** `TFT_eSPI` with `TFT_eSprite` double-buffering for flicker-free partial screen updates
* **Input:** Resistive/Capacitive Touch via `tft.getTouch(&tx, &ty)` (easily swappable with physical GPIO buttons)
* **Networking (device):** `WiFi.h` + `HTTPClient.h`; requests run in a FreeRTOS task so the UI never blocks
* **Time source:** NTP (`configTime`, UTC) once Wi-Fi connects; used by the countdown screen
* **Model host:** 2020 13" MacBook Pro (Intel) running Ollama with `qwen3.5:9b` (CPU inference, replies take several seconds or more)
* **Bridge service:** Python `FastAPI` + `uvicorn` + `httpx` (`server.py`) with native Ollama tool/function calling
* **Color Palette:**
  * Background: `TFT_BLACK` (`0x0000`)
  * Primary Phosphor Green: `CODEC_GREEN` (`0x56AD`)
  * Highlight / Active Accent: `CODEC_BRIGHT` (`0x87F0`)
  * Dim Grid / Sub-frame Accent: `CODEC_DIM` (`0x2345`)
  * Umbrella Red (countdown screen only): `COLOR_UMBRELLA_RED` (`0xF800`), the only non-green color in the UI

---

## Current Architecture & Features

### 1. Persistent Header & Wi-Fi Status Globe (`y = 0..52`)
* **Static Branding:** Displays the custom `160×36` pixel `PROGMEM` bitmap (`schunet_logo.h`) centered at the top of the screen with a horizontal CRT scanline overlay.
* **Connection-Gated Animation:**
  * **Disconnected (`WiFi.status() != WL_CONNECTED`):** Renders the original static wireframe globe bitmap.
  * **Connected (`WiFi.status() == WL_CONNECTED`):** Smoothly rotates the `36×36` globe region at ~33 FPS using an off-screen `TFT_eSprite` buffer and non-blocking `millis()` timing, without redrawing the rest of the screen.

### 2. Extensible Screen & Button Navigation Engine
The UI below the header (`y = 54..320`) is driven by a clean state machine (`enum ScreenID`) and a declarative button struct (`CodecButton`):
* **`ScreenID` State Machine:** Tracks `currentScreen` (`SCREEN_HOME`, `SCREEN_AI_AGENT`, etc.). Calling `navigateTo(ScreenID nextScreen)` clears only the content viewport below the header (`y = 54` to `320`) and renders the target view.
* **Declarative Button System (`CodecButton`):**
  * Stores button bounds (`x, y, w, h`), label text, destination `ScreenID`, and visibility state.
  * Styled with Codec corner brackets, double-line borders, and scanline shading.
  * Buttons with an empty label are treated as icon-only (the frame is drawn and the caller draws the icon on top).
  * Adding future menu options (e.g., *System Status*, *Network Scanner*, *Settings*) only requires adding an enum entry, appending a `CodecButton` to `homeButtons[]`, and adding a `draw<ScreenName>()` handler.

### 3. Screens Implemented
* **Home Menu (`SCREEN_HOME`):**
  * Renders two Codec action buttons: **"AI AGENT"** and **"RELEASE COUNTDOWN"** (temporary).
  * Tapping a button hides the home menu and transitions to its target screen (`SCREEN_AI_AGENT` or `SCREEN_COUNTDOWN`).
* **AI Agent Codec Screen (`SCREEN_AI_AGENT`):**
  * Displays an MGS Codec-style **portrait frame** (`96×112` px) containing a stylized, phosphor-green cybernetic/tactical agent bust with scanlines and frequency bars (`FREQ 140.85`).
  * **Eye Animation (`updateAgentEyes()`):** Random blinks (including occasional double-blinks) and subtle left/center/right gaze shifts, rendered through a small `34×14` eye sprite.
  * **Lip-Sync Mouth Animation (`updateAgentSpeech()` / `drawAgentMouthFrame()`):** While a reply is actively typing out, a dedicated `24×10` mouth sprite cycles through closed, half-open, and open jaw states with aligned CRT scanlines, pausing cleanly during spaces and punctuation before snapping shut once the message finishes.
  * **Microphone button** (`micButton`) sits to the left of the portrait, mirroring the signal-strength bars on the right. Its icon is drawn with primitives and dims while a request is in flight.
  * **Typewriter Dialogue Box (`prepareDialogueLines()` + `updateAgentSpeech()`):** Pre-wraps replies up to 4 lines (truncating overflow with `...`) and reveals characters one by one (~35 ms per character, with brief pauses on punctuation) in `loop()`. If the user navigates away mid-stream and returns, the full `lastDialogue` is shown immediately.
  * A **`< BACK`** button returns to the main menu. If a request is still running, its reply is kept and shown when the user returns to this screen.
* **Release Countdown Screen (`SCREEN_COUNTDOWN`, temporary):** a rocket flying through space with a red umbrella on it, a title, and a live countdown. Details in section 6.

### 4. LLM Chat Flow (device side)
1. User taps the mic button → `startChatRequest()` (ignored if a request is already pending).
2. `chatState` moves `CHAT_IDLE → CHAT_PENDING`; any active typewriter stream stops, the mouth closes, the mic icon dims, and the dialogue box is cleared.
3. A FreeRTOS task (`chatTaskFn`, pinned to core 0, 10 KB stack) sends `POST {CHAT_URL}` with body `{}`, so the server uses its temporary default prompt.
4. While pending, `updateChatRequest()` (called from `loop()` on core 1) animates **three bouncing dots** in the dialogue box every 40 ms through a `60×30` sprite (flicker-free).
5. When the task finishes it writes `chatResult`, then flips `chatState` to `CHAT_DONE` or `CHAT_ERROR`. The main loop restores the mic icon and kicks off `startAgentSpeech(lastDialogue)` to typewriter-print the reply while moving the agent's mouth.

Details worth knowing:
* **Response parsing:** `extractReply()` is a small hand-written parser for `{"reply": "..."}` (no JSON library needed). It handles JSON escapes and UTF-8.
* **ASCII folding:** the built-in TFT font is ASCII-only, so curly quotes, en/em dashes, and ellipses are converted to plain equivalents and other non-ASCII characters (emoji, accents) are dropped.
* **Error messages shown in the box:** `NO WIFI LINK`, `BAD URL`, `SERVER ERROR <code>`, `LINK FAILED: <reason>`, `EMPTY RESPONSE`.
* **Timeout:** `HTTPClient::setTimeout()` takes a `uint16_t` (ms), so the practical maximum is ~65 s. The sketch uses 60 s (`CHAT_TIMEOUT_MS`).
* **Configuration:** set `CHAT_URL` (e.g. `http://192.168.1.50:8000/chat`) via `secrets.h` (`CHAT_SERVER_HOST` / `CHAT_SERVER_PORT`).

### 5. Model Bridge Service & Tool Calling (`server.py`)
A small FastAPI app that sits between the ESP32 and Ollama, handling LLM inference and executing external API tools.

| Endpoint | Method | Purpose |
|---|---|---|
| `/health` | GET | Returns `{"ok": true, "model": ...}` for connectivity checks |
| `/chat` | POST | Blocking call. Optional JSON body `{"prompt": "..."}`; falls back to a temporary hardcoded default prompt. Returns `{"reply": "..."}` |
| `/jobs` | POST | Submit-and-poll variant. Returns `{"id": "..."}` immediately and generates in the background |
| `/jobs/{id}` | GET | Returns `{"status": "pending" \| "done" \| "error", "reply": ...}` |

#### Tool-Calling Architecture & ESPN Scoreboard Validation
To let `qwen3.5:9b` fetch live external data without overflowing context limits on CPU, `run_model()` implements a two-step tool pipeline:
1. **Tool Selection Pass (`think: True`):** Calls `/api/chat` with `tools: TOOLS`, a neutral tool-focused system prompt (`TOOL_SYSTEM_PROMPT`), and `think: True` (`num_predict: 512`). Enabling Qwen's reasoning tokens on this first pass ensures the model reliably emits structured `tool_calls` instead of roleplaying answers in plain text.
2. **Tool Execution & JSON Condensing:** When `tool_calls` are returned, `server.py` executes the requested Python function, filters the raw API payload into a compact plain-text summary (keeping prompt evaluation fast on CPU), and appends the `role: "tool"` message.
3. **Screen Formatting Pass (`think: False`):** Swaps in `SCREEN_SYSTEM_PROMPT` and runs a fast non-thinking pass (`num_predict: 80`) to format the final answer in plain text, under 25 words, with no markdown or emoji.

**First Tool Implemented (`get_nfl_scoreboard`):**
* **Implementation:** Queries `https://site.web.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard` (with a browser `User-Agent` and `follow_redirects=False` so ESPN returns raw JSON rather than redirecting to HTML) and extracts each matchup's team names, scores, and clock/quarter status.
* **Validation & Scope:** Successfully tested end-to-end on the ESP32 using a **temporary** hardcoded `DEFAULT_PROMPT` querying a single NFL matchup (*Buffalo Bills vs. Kansas City Chiefs*). While the single-game hardcoded prompt is strictly temporary for initial validation, the `get_nfl_scoreboard` tool itself is reusable for any NFL game or dynamic query, and the `TOOLS` registry establishes the pattern for adding more tools in the future.

Behavior and tuning:
* **Keep-alive:** `keep_alive: "30m"` keeps the model loaded in memory between taps.
* **Serialization:** an `asyncio.Lock` ensures only one generation runs at a time.
* **Environment variables:** `OLLAMA_URL` (default `http://127.0.0.1:11434`) and `MODEL` (default `qwen3.5:9b`).

Run it:
```bash
python3 -m venv .venv && source .venv/bin/activate
pip install fastapi uvicorn httpx
uvicorn server:app --host 0.0.0.0 --port 8000
```
`--host 0.0.0.0` is required so the ESP32 can reach it. Find the laptop IP with `ipconfig getifaddr en0`, allow incoming connections if macOS prompts, and consider a DHCP reservation so the address stays stable.

Quick tests:
```bash
curl http://<laptop-ip>:8000/health
curl -X POST http://<laptop-ip>:8000/chat
curl -X POST http://<laptop-ip>:8000/jobs
curl http://<laptop-ip>:8000/jobs/<id>
```

### 6. Release Countdown Screen (temporary)
A novelty screen counting down to **8:00 AM EST on Dec 2, 2026** (13:00 UTC, Unix epoch `1796216400`, stored as `COUNTDOWN_TARGET_EPOCH`). It is not intended to be a long-term app and should be retired after the release.

* **Title:** "Assisted Voice Auto" / "Production Release" on two lines above the rocket, plus a `T-MINUS` tag by the back button.
* **Rocket (`drawRocket()`):** vector-style rocket in the codec greens (filled ogive nose cone with porthole, collar band, shaded body, swept fins, nozzle) inside a double frame with CRT scanlines.
* **Umbrella (`drawUmbrella()`):** red umbrella on the rocket body, inspired by the Travelers Insurance logo (wide dome canopy, three scallops, curved ribs, pointed tip, shaft with a J-shaped handle). It is drawn after the scanlines so it stays solid, and it is the only red element in the UI.
* **Flying through space:** `updateStarfield()` animates 28 stars streaking downward at 1 to 4 px per frame (faster stars are brighter and longer, for parallax). Stars skip a rocket silhouette mask and only draw on even rows so the scanline effect stays intact. `updateRocketFlame()` flickers a green engine flame every ~110 ms.
* **Countdown boxes:** DAYS / HRS / MIN / SEC, drawn in four codec-style boxes. Only digits that change are redrawn (via `setTextPadding`), so there is no flicker.
* **Clock:** the ESP32 has no RTC, so time comes from NTP, started once from `loop()` when Wi-Fi first connects. Before the clock syncs the boxes show `--`; at zero they hold at `00`. The target is a UTC epoch, so no timezone handling is needed.

### 7. Touch Handling
* Touch is polled in `handleInputs()` with a 250 ms debounce.
* Hit testing is two-pass: exact hits (`containsDirect()`, 6 px pad) are tried first, then the legacy mirrored hit test (`contains()`, 12 px pad) as a fallback. Mirroring cannot distinguish multiple buttons, so exact hits must win.
* If the wrong button fires, set `TOUCH_DEBUG` to `1` to print raw touch coordinates on the serial monitor. A proper `tft.setTouch()` calibration is the long-term fix.
* Serial shortcuts: `b` goes home; `1` opens the AI Agent screen and `2` opens the countdown (from the home screen).

---

## File Structure
```text
├── schunet-client-2.7/schunet-client-2.7.ino  # Main application state machine, Wi-Fi loop, UI renderer, touch handler, chat client
├── schunet-client-2.7/schunet_logo.h        # 160x36 1-bit PROGMEM bitmap array for the SCHUNET header logo
├── schunet-client-2.7/secrets.h             # secrets file (not tracked in source control) - a placeholder file also exists named secrets-placeholder.h, this file is tracked in source control
├── server/server.py                          # FastAPI bridge between the ESP32 and the local Ollama model (with tool execution)
└── PROJECT_OVERVIEW.md                       # System architecture and feature documentation
```

---

## Known Limitations
* The prompt is currently hardcoded on the server as a temporary test query for a single NFL game; the mic button is only a trigger and does not capture audio.
* Only one external tool (`get_nfl_scoreboard`) is currently registered in `server.py`.
* The dialogue box shows at most 4 lines; longer replies are truncated with `...`.
* `/chat` is a single blocking request capped at ~65 s by `HTTPClient`; slow generations (e.g. a cold model load or multi-pass tool execution on the Intel CPU) can approach or exceed this timeout.
* Replies are received in full from the server before typewriter playback begins on screen (true token-by-token HTTP streaming is not yet implemented).
* Wi-Fi credentials are stored in `secrets.h` (use `secrets-placeholder.h` as a template when cloning).
* The countdown needs Wi-Fi and NTP to show real values, and the screen itself is temporary.

---

## Roadmap / Next Steps
**Time-boxed:** the release countdown screen is a temporary novelty. Plan to remove it after Dec 2, 2026 (item 7).

1. **Prompt input:** replace the temporary hardcoded single-game prompt (options under consideration: on-screen touch keyboard, preset prompt buttons, a phone/browser page that posts text to the server, serial monitor input for development, or real speech-to-text) so users can query any NFL game or general topic dynamically.
2. **Expand LLM Tools:** build on the `get_nfl_scoreboard` pattern in `server.py` to add more tools (e.g., additional sports leagues, local weather, calendar/reminders, or system telemetry) and short-TTL caching for external API responses.
3. **Long-running requests:** switch the device to the `/jobs` submit-and-poll endpoints (already implemented on the server) if two-pass tool replies regularly exceed the ~65 s timeout, or move to WebSockets.
4. **End-to-end streaming:** stream tokens directly from the server during generation (building on the existing on-device typewriter renderer); consider paging or scrolling for longer replies.
5. **Additional Navigation Modules:** Expand `ScreenID` and `homeButtons[]` with new screens while keeping the top SCHUNET header locked in place.
6. **Audio & Codec Effects:** Integrate an I2S DAC/amp (such as MAX98357A) or piezo buzzer for authentic Codec call chimes and text-bleep sound effects.
7. **Retire the countdown screen after 12/2/26:** remove `SCREEN_COUNTDOWN` from `enum ScreenID`, the "RELEASE COUNTDOWN" entry in `homeButtons[]`, its `case` in `navigateTo()`, and the serial `2` shortcut. Delete the countdown code block (`drawUmbrella`, `drawRocket`, `updateRocketFlame`, `updateStarfield`, `inRocketMask`, `respawnStar`, `drawStar`, `countdownBoxX`, `drawCountdownBoxes`, `drawCountdownValue`, `updateCountdown`, `drawCountdownScreen`), the related globals and `#define`s (`COLOR_UMBRELLA_RED`, `COUNTDOWN_TARGET_EPOCH`, `ROCKET_*`, `CD_BOX_*`, `FLAME_*`, `STAR_*`, `NUM_STARS`), and the `updateCountdown()`, `updateRocketFlame()` and `updateStarfield()` calls in `loop()`. The NTP setup (`configTime`) can stay if you want a clock for future features.

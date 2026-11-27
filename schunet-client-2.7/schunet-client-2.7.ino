
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <TFT_eSPI.h>
#include <math.h>
#include "schunet_logo.h"
#include "secrets.h"

// ==========================================
// 1. CONFIGURATION & COLOR PALETTE
// ==========================================
// URL of the laptop running the FastAPI/Ollama bridge (find IP with: ipconfig getifaddr en0)
#define STR_(x) #x
#define STR(x) STR_(x)
#define CHAT_URL "http://" CHAT_SERVER_HOST ":" STR(CHAT_SERVER_PORT) "/chat"
// NOTE: HTTPClient::setTimeout() is a uint16_t (ms), so ~65s is the maximum.
#define CHAT_TIMEOUT_MS 60000

#define CODEC_GREEN 0x56AD   // Primary phosphor green
#define CODEC_BRIGHT 0x87F0  // Highlight / crisp text green
#define CODEC_DIM 0x2345     // Subtle grid / shadow green

// Umbrella red: the ONLY non-green color in the UI
#define COLOR_UMBRELLA_RED 0xF800

// Countdown target: 8:00 AM EST on Dec 2, 2026. EST = UTC-5, so that is 13:00 UTC.
// (Unix epoch 1796216400; clock comes from NTP once Wi-Fi connects.)
#define COUNTDOWN_TARGET_EPOCH 1796216400UL

// Set to 1 to print raw touch coordinates on the serial monitor
#define TOUCH_DEBUG 0

// Countdown screen geometry
#define ROCKET_FRAME_X 11   // lines up with the countdown boxes below
#define ROCKET_FRAME_Y 132
#define ROCKET_FRAME_W 218
#define ROCKET_FRAME_H 130
#define CD_BOX_Y 266
#define CD_BOX_W 50
#define CD_BOX_H 48
#define CD_BOX_GAP 6
#define FLAME_X 106
#define FLAME_Y 245  // keep odd so it lines up with the CRT scanlines
#define FLAME_W 29
#define FLAME_H 13

// Persistent Header Geometry
#define HEADER_HEIGHT 54
#define GLOBE_SIZE 36
#define GLOBE_RADIUS 15

// Agent Portrait & Eye Sprite Geometry
#define PORTRAIT_W 96
#define PORTRAIT_H 112
#define EYE_BOX_W 34
#define EYE_BOX_H 14

// Dialogue box geometry (y/h fixed, x/w derived from screen width)
#define DLG_X 12
#define DLG_Y 216
#define DLG_H 92
#define DLG_MAX_LINES 4

// Bouncing dots sprite (centered inside dialogue box)
#define DOTS_W 60
#define DOTS_H 30
#define DOTS_INTERVAL 40

// ==========================================
// 2. EXTENSIBLE NAVIGATION & UI STRUCTS
// ==========================================
enum ScreenID {
  SCREEN_HOME,
  SCREEN_AI_AGENT,
  SCREEN_COUNTDOWN
  // Add future screens here (e.g., SCREEN_SYS_INFO, SCREEN_NET_SCAN)
};

struct CodecButton {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
  const char* label;
  ScreenID targetScreen;
  bool visible;

  bool hitBox(int16_t px, int16_t py, int16_t pad = 12) const {
    return (px >= (x - pad) && px <= (x + w + pad) && py >= (y - pad) && py <= (y + h + pad));
  }

  // Exact (slightly padded) hit test with no mirroring; tried first when there are several buttons
  bool containsDirect(int16_t tx, int16_t ty) const {
    return visible && hitBox(tx, ty, 6);
  }

  bool contains(int16_t tx, int16_t ty, int16_t screenW, int16_t screenH) const {
    if (!visible) return false;
    return hitBox(tx, ty) || hitBox(screenW - tx, screenH - ty) || hitBox(tx, screenH - ty) || hitBox(screenW - tx, ty);
  }
};

CodecButton homeButtons[] = {
  { 35, 110, 170, 42, "AI AGENT", SCREEN_AI_AGENT, true },
  { 35, 166, 170, 42, "RELEASE COUNTDOWN", SCREEN_COUNTDOWN, true }
};
const int NUM_HOME_BUTTONS = sizeof(homeButtons) / sizeof(homeButtons[0]);

CodecButton backButton = { 10, 60, 82, 28, "< BACK", SCREEN_HOME, false };

// Microphone button, left of the portrait (portrait is at x=72..168, y=94..206)
CodecButton micButton = { 20, 128, 44, 44, "", SCREEN_AI_AGENT, false };

// ==========================================
// 3. GLOBALS & ANIMATION STATE
// ==========================================
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite globeSprite = TFT_eSprite(&tft);
TFT_eSprite eyeSprite = TFT_eSprite(&tft);
TFT_eSprite dotsSprite = TFT_eSprite(&tft);

ScreenID currentScreen = SCREEN_HOME;
int16_t logoX, logoY;
int16_t portraitX, portraitY;

// Globe rotation state
float globePhase = 0.0;
unsigned long lastAnimTime = 0;
const unsigned long ANIM_INTERVAL = 30;
bool wasConnected = false;

// Touch debounce
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE = 250;

// Agent Eye Animation State
int8_t eyeLookOffset = 0;  // -2 (left), 0 (center), +2 (right)
bool eyesClosed = false;
unsigned long nextBlinkTime = 0;
unsigned long blinkEndTime = 0;
unsigned long nextLookTime = 0;

// Chat request state (request runs in a background task so animations keep going)
enum ChatState { CHAT_IDLE, CHAT_PENDING, CHAT_DONE, CHAT_ERROR };
volatile ChatState chatState = CHAT_IDLE;
String chatResult;  // written by the task BEFORE chatState flips to DONE/ERROR
String lastDialogue = "CODEC LINK READY.\nAWAITING PROMPT...";

// Bouncing dots animation
float dotsPhase = 0.0;
unsigned long lastDotsTime = 0;

// Countdown screen state
bool ntpStarted = false;
long lastCdValues[4] = { -2, -2, -2, -2 };
unsigned long lastCdCheck = 0;
unsigned long lastFlameTime = 0;

// ==========================================
// 4. PERSISTENT HEADER & GLOBE RENDERING
// ==========================================
void applyScanlines(int16_t x, int16_t y, int16_t w, int16_t h) {
  for (int16_t row = y; row < y + h; row += 2) {
    tft.drawFastHLine(x, row, w, TFT_BLACK);
  }
}

void drawStaticHeader() {
  tft.fillRect(0, 0, tft.width(), HEADER_HEIGHT, TFT_BLACK);
  tft.drawBitmap(logoX, logoY, schunet_logo, SCHUNET_LOGO_WIDTH, SCHUNET_LOGO_HEIGHT, CODEC_GREEN, TFT_BLACK);
  applyScanlines(logoX, logoY, SCHUNET_LOGO_WIDTH, SCHUNET_LOGO_HEIGHT);

  tft.drawFastHLine(10, HEADER_HEIGHT - 2, tft.width() - 20, CODEC_DIM);
  tft.drawFastHLine(10, HEADER_HEIGHT - 1, 24, CODEC_GREEN);
  tft.drawFastHLine(tft.width() - 34, HEADER_HEIGHT - 1, 24, CODEC_GREEN);
}

void drawSpinningGlobe(float angleOffset) {
  globeSprite.fillSprite(TFT_BLACK);
  int cx = 17;
  int cy = 17;

  globeSprite.drawCircle(cx, cy, GLOBE_RADIUS, CODEC_GREEN);
  globeSprite.drawFastVLine(cx, cy - GLOBE_RADIUS, GLOBE_RADIUS * 2, CODEC_GREEN);

  globeSprite.drawFastHLine(cx - GLOBE_RADIUS, cy, GLOBE_RADIUS * 2, CODEC_GREEN);
  globeSprite.drawFastHLine(cx - 13, cy - 7, 26, CODEC_GREEN);
  globeSprite.drawFastHLine(cx - 13, cy + 7, 26, CODEC_GREEN);
  globeSprite.drawFastHLine(cx - 9, cy - 12, 18, CODEC_GREEN);
  globeSprite.drawFastHLine(cx - 9, cy + 12, 18, CODEC_GREEN);

  for (int i = 0; i < 3; i++) {
    float theta = angleOffset + (i * PI / 3.0);
    int rx = abs(round(GLOBE_RADIUS * sin(theta)));
    if (rx > 0) {
      globeSprite.drawEllipse(cx, cy, rx, GLOBE_RADIUS, CODEC_GREEN);
    }
  }

  for (int y = 0; y < GLOBE_SIZE; y += 2) {
    globeSprite.drawFastHLine(0, y, GLOBE_SIZE, TFT_BLACK);
  }

  globeSprite.pushSprite(logoX, logoY);
}

// ==========================================
// 5. CODEC UI COMPONENTS & SCREENS
// ==========================================
void drawCodecButton(const CodecButton& btn) {
  if (!btn.visible) return;

  tft.fillRect(btn.x, btn.y, btn.w, btn.h, TFT_BLACK);
  tft.drawRect(btn.x, btn.y, btn.w, btn.h, CODEC_GREEN);
  tft.drawRect(btn.x + 2, btn.y + 2, btn.w - 4, btn.h - 4, CODEC_DIM);

  int bLen = 6;
  tft.drawFastHLine(btn.x - 1, btn.y - 1, bLen, CODEC_BRIGHT);
  tft.drawFastVLine(btn.x - 1, btn.y - 1, bLen, CODEC_BRIGHT);
  tft.drawFastHLine(btn.x + btn.w - bLen + 1, btn.y + btn.h, bLen, CODEC_BRIGHT);
  tft.drawFastVLine(btn.x + btn.w, btn.y + btn.h - bLen + 1, bLen, CODEC_BRIGHT);

  if (btn.label[0] == '\0') return;  // icon-only button
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(CODEC_BRIGHT, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(btn.label, btn.x + (btn.w / 2), btn.y + (btn.h / 2));
}

// Microphone button: codec frame + mic icon (dimmed while a request is in flight)
void drawMicButton() {
  if (!micButton.visible) return;
  drawCodecButton(micButton);

  uint16_t col = (chatState == CHAT_PENDING) ? CODEC_DIM : CODEC_BRIGHT;
  int cx = micButton.x + micButton.w / 2;
  int y = micButton.y;

  tft.fillRoundRect(cx - 5, y + 8, 10, 17, 5, col);      // capsule
  tft.drawCircleHelper(cx, y + 21, 9, 0x0C, col);        // U-shaped holder (bottom half)
  tft.drawFastVLine(cx - 9, y + 16, 6, col);
  tft.drawFastVLine(cx + 9, y + 16, 6, col);
  tft.drawFastVLine(cx, y + 30, 4, col);                 // stem
  tft.drawFastHLine(cx - 6, y + 34, 13, col);            // base
}

// Renders the animated eye/brow region into eyeSprite and pushes to portrait
void drawAgentEyesFrame(int8_t lookOffset, bool closed) {
  int cx = portraitX + PORTRAIT_W / 2;
  int cy = portraitY + PORTRAIT_H / 2 - 4;

  // Top-left corner of the 34x14 eye region on screen
  int16_t ex = cx - (EYE_BOX_W / 2);
  int16_t ey = cy - 11;

  // Local center inside eyeSprite
  int scx = EYE_BOX_W / 2;
  int scy = 11;

  eyeSprite.fillSprite(TFT_BLACK);

  // Restore subtle vertical portrait background grid inside the eye box
  for (int gx = portraitX + 12; gx < portraitX + PORTRAIT_W - 8; gx += 16) {
    if (gx >= ex && gx < ex + EYE_BOX_W) {
      eyeSprite.drawFastVLine(gx - ex, 0, EYE_BOX_H, CODEC_DIM);
    }
  }

  // Angled eyebrows (slight expressive dip when blinking)
  int browDip = closed ? 1 : 0;
  eyeSprite.drawLine(scx - 14, scy - 10 + browDip, scx - 4, scy - 8 + browDip, CODEC_BRIGHT);
  eyeSprite.drawLine(scx + 14, scy - 10 + browDip, scx + 4, scy - 8 + browDip, CODEC_BRIGHT);

  // Nose bridge top between eyes
  eyeSprite.drawFastVLine(scx, scy - 7, 10, CODEC_GREEN);
  eyeSprite.drawFastVLine(scx - 1, scy - 3, 6, CODEC_DIM);

  if (closed) {
    // Closed eyelid crease
    eyeSprite.drawFastHLine(scx - 13, scy - 5, 9, CODEC_DIM);
    eyeSprite.drawFastHLine(scx + 5, scy - 5, 9, CODEC_DIM);
    eyeSprite.drawFastHLine(scx - 13, scy - 4, 9, CODEC_BRIGHT);
    eyeSprite.drawFastHLine(scx + 5, scy - 4, 9, CODEC_BRIGHT);
  } else {
    // Deep brow socket shadow
    eyeSprite.drawFastHLine(scx - 13, scy - 7, 9, CODEC_DIM);
    eyeSprite.drawFastHLine(scx + 5, scy - 7, 9, CODEC_DIM);

    // Upper eyelids
    eyeSprite.drawFastHLine(scx - 13, scy - 6, 9, CODEC_GREEN);
    eyeSprite.drawFastHLine(scx + 5, scy - 6, 9, CODEC_GREEN);

    // Eye openings (7x3 crisp shape)
    eyeSprite.fillRect(scx - 12, scy - 5, 7, 3, CODEC_GREEN);
    eyeSprite.fillRect(scx + 6, scy - 5, 7, 3, CODEC_GREEN);

    // Shifted pupils + bright phosphor catchlights
    int pLeftX = scx - 10 + lookOffset;
    int pRightX = scx + 8 + lookOffset;

    eyeSprite.fillRect(pLeftX, scy - 5, 3, 3, TFT_BLACK);
    eyeSprite.fillRect(pRightX, scy - 5, 3, 3, TFT_BLACK);
    eyeSprite.drawPixel(pLeftX, scy - 5, CODEC_BRIGHT);
    eyeSprite.drawPixel(pRightX, scy - 5, CODEC_BRIGHT);

    // Lower eyelid crease
    eyeSprite.drawFastHLine(scx - 12, scy - 2, 7, CODEC_DIM);
    eyeSprite.drawFastHLine(scx + 6, scy - 2, 7, CODEC_DIM);
  }

  // Match CRT scanlines aligned to screen Y coordinates
  for (int y = 0; y < EYE_BOX_H; y++) {
    if (((ey + y) % 2) == 1) {
      eyeSprite.drawFastHLine(0, y, EYE_BOX_W, TFT_BLACK);
    }
  }

  eyeSprite.pushSprite(ex, ey);
}

void drawAgentPortrait(int16_t px, int16_t py, int16_t pw, int16_t ph) {
  tft.drawRect(px, py, pw, ph, CODEC_GREEN);
  tft.drawRect(px + 2, py + 2, pw - 4, ph - 4, CODEC_DIM);

  int cx = px + pw / 2;
  int cy = py + ph / 2 - 4;

  // Background grid lines
  for (int gx = px + 12; gx < px + pw - 8; gx += 16) {
    tft.drawFastVLine(gx, py + 4, ph - 8, CODEC_DIM);
  }
  for (int gy = py + 12; gy < py + ph - 8; gy += 16) {
    tft.drawFastHLine(px + 4, gy, pw - 8, CODEC_DIM);
  }

  // Shoulders / Tactical Collar
  tft.drawLine(cx - 34, py + ph - 4, cx - 24, cy + 28, CODEC_GREEN);
  tft.drawLine(cx + 34, py + ph - 4, cx + 24, cy + 28, CODEC_GREEN);
  tft.drawLine(cx - 24, cy + 28, cx - 12, cy + 34, CODEC_GREEN);
  tft.drawLine(cx + 24, cy + 28, cx + 12, cy + 34, CODEC_GREEN);
  tft.drawLine(cx - 12, cy + 34, cx + 12, cy + 34, CODEC_BRIGHT);

  // Neck
  tft.drawFastVLine(cx - 10, cy + 18, 12, CODEC_GREEN);
  tft.drawFastVLine(cx + 10, cy + 18, 12, CODEC_GREEN);
  tft.drawLine(cx - 5, cy + 21, cx - 3, cy + 30, CODEC_DIM);
  tft.drawLine(cx + 5, cy + 21, cx + 3, cy + 30, CODEC_DIM);

  // Jaw & Head Silhouette
  tft.drawLine(cx - 18, cy - 16, cx - 18, cy + 4, CODEC_BRIGHT);
  tft.drawLine(cx + 18, cy - 16, cx + 18, cy + 4, CODEC_BRIGHT);
  tft.drawLine(cx - 18, cy + 4, cx - 9, cy + 20, CODEC_BRIGHT);
  tft.drawLine(cx + 18, cy + 4, cx + 9, cy + 20, CODEC_BRIGHT);
  tft.drawLine(cx - 9, cy + 20, cx + 9, cy + 20, CODEC_BRIGHT);

  // Cheekbone shading
  tft.drawLine(cx - 15, cy + 2, cx - 11, cy + 8, CODEC_DIM);
  tft.drawLine(cx + 15, cy + 2, cx + 11, cy + 8, CODEC_DIM);

  // Hair / Tactical Headband
  tft.fillRect(cx - 19, cy - 22, 39, 6, CODEC_GREEN);
  tft.drawLine(cx - 19, cy - 22, cx - 10, cy - 30, CODEC_GREEN);
  tft.drawLine(cx - 10, cy - 30, cx + 12, cy - 30, CODEC_GREEN);
  tft.drawLine(cx + 12, cy - 30, cx + 19, cy - 22, CODEC_GREEN);
  tft.drawLine(cx + 19, cy - 20, cx + 27, cy - 14, CODEC_GREEN);
  tft.drawLine(cx + 19, cy - 17, cx + 25, cy - 10, CODEC_GREEN);
  // Lower nose, mouth & Codec earpiece
  tft.drawFastVLine(cx, cy - 7, 12, CODEC_GREEN);
  tft.drawFastVLine(cx - 1, cy - 3, 8, CODEC_DIM);
  tft.drawFastHLine(cx - 2, cy + 5, 5, CODEC_BRIGHT);
  tft.drawFastHLine(cx - 6, cy + 12, 13, CODEC_BRIGHT);
  tft.drawFastHLine(cx - 3, cy + 14, 7, CODEC_DIM);
  tft.drawRect(cx - 21, cy - 4, 3, 10, CODEC_BRIGHT);
  // Apply scanlines across the static portrait base
  applyScanlines(px + 1, py + 1, pw - 2, ph - 2);
  // Draw initial open eyes
  drawAgentEyesFrame(eyeLookOffset, eyesClosed);
}

void updateAgentEyes() {
  if (currentScreen != SCREEN_AI_AGENT) return;
  unsigned long now = millis();
  bool needsRedraw = false;
  // 1. Handle Blink Cycle
  if (!eyesClosed && now >= nextBlinkTime) {
    eyesClosed = true;
    blinkEndTime = now + 115;  // Eyelids stay closed for 115ms
    needsRedraw = true;
  } else if (eyesClosed && now >= blinkEndTime) {
    eyesClosed = false;
    // Schedule next blink in 2.8 to 5.5 seconds (or a quick double-blink 20% of the time)
    if (random(0, 5) == 0) {
      nextBlinkTime = now + 220;
    } else {
      nextBlinkTime = now + random(2800, 5500);
    }
    needsRedraw = true;
  }
  // 2. Handle Subtle Looking Around (Left / Center / Right)
  if (now >= nextLookTime) {
    int8_t choices[] = { -2, 0, 0, 2 };  // Weighted toward looking forward
    int8_t nextOffset = choices[random(0, 4)];
    if (nextOffset != eyeLookOffset) {
      eyeLookOffset = nextOffset;
      if (!eyesClosed) needsRedraw = true;
    }
    nextLookTime = now + random(1800, 4000);
  }
  if (needsRedraw) {
    drawAgentEyesFrame(eyeLookOffset, eyesClosed);
  }
}

// Clears the dialogue box and redraws its double border
void drawDialogueFrame() {
  int16_t bw = tft.width() - 24;
  tft.fillRect(DLG_X, DLG_Y, bw, DLG_H, TFT_BLACK);
  tft.drawRect(DLG_X, DLG_Y, bw, DLG_H, CODEC_GREEN);
  tft.drawRect(DLG_X + 2, DLG_Y + 2, bw - 4, DLG_H - 4, CODEC_DIM);
}

// Word-wrapped text inside the dialogue box. Limited to DLG_MAX_LINES; overflow ends in "..."
void drawAgentDialogue(const String& text) {
  drawDialogueFrame();

  int16_t bw = tft.width() - 24;
  int16_t curX = DLG_X + 10;
  int16_t curY = DLG_Y + 10;
  int16_t maxW = bw - 20;
  int16_t lineH = 18;

  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(CODEC_BRIGHT, TFT_BLACK);

  String lines[DLG_MAX_LINES];
  int n = 0;
  bool truncated = false;
  String line = "";
  String word = "";

  auto pushLine = [&](const String& l) -> bool {
    if (n >= DLG_MAX_LINES) {
      truncated = true;
      return false;
    }
    lines[n++] = l;
    return true;
  };

  for (unsigned int i = 0; i <= text.length() && !truncated; i++) {
    char c = (i < text.length()) ? text[i] : '\n';
    if (c == ' ' || c == '\n') {
      if (word.length() > 0) {
        String testLine = (line.length() > 0) ? (line + " " + word) : word;
        if (line.length() > 0 && tft.textWidth(testLine, 2) > maxW) {
          if (!pushLine(line)) break;
          line = word;
        } else {
          line = testLine;
        }
        word = "";
      }
      if (c == '\n' && i < text.length()) {  // explicit newline in the text
        if (!pushLine(line)) break;
        line = "";
      }
    } else {
      word += c;
    }
  }
  if (!truncated && line.length() > 0) pushLine(line);

  if (truncated && n > 0) {
    String& last = lines[n - 1];
    while (last.length() > 0 && tft.textWidth(last + "...", 2) > maxW) {
      last.remove(last.length() - 1);
    }
    last += "...";
  }

  for (int k = 0; k < n; k++) {
    tft.drawString(lines[k], curX, curY + k * lineH, 2);
  }
}

// One frame of the three bouncing dots, drawn via sprite to avoid flicker
void drawBouncingDots() {
  dotsSprite.fillSprite(TFT_BLACK);
  for (int i = 0; i < 3; i++) {
    float bounce = fabs(sin(dotsPhase - i * 0.6));  // 0..1, ball-like hops, staggered per dot
    int dy = (int)round(bounce * 14);
    dotsSprite.fillCircle(10 + i * 20, 24 - dy, 4, CODEC_BRIGHT);
  }
  int16_t bw = tft.width() - 24;
  dotsSprite.pushSprite(DLG_X + (bw - DOTS_W) / 2, DLG_Y + (DLG_H - DOTS_H) / 2);
}

void drawHomeScreen() {
  backButton.visible = false;
  micButton.visible = false;
  for (int i = 0; i < NUM_HOME_BUTTONS; i++) {
    homeButtons[i].visible = true;
    drawCodecButton(homeButtons[i]);
  }
}

void drawAIAgentScreen() {
  for (int i = 0; i < NUM_HOME_BUTTONS; i++) {
    homeButtons[i].visible = false;
  }
  backButton.visible = true;
  drawCodecButton(backButton);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(CODEC_BRIGHT, TFT_BLACK);
  tft.drawString("FREQ 140.85", tft.width() - 14, 66);
  portraitX = (tft.width() - PORTRAIT_W) / 2;
  portraitY = 94;
  // Reset eye state on entry
  eyeLookOffset = 0;
  eyesClosed = false;
  nextBlinkTime = millis() + 1800;
  nextLookTime = millis() + 2400;
  drawAgentPortrait(portraitX, portraitY, PORTRAIT_W, PORTRAIT_H);
  // Signal strength bars on right of portrait
  for (int b = 0; b < 5; b++) {
    int barW = 6 + (b * 4);
    uint16_t col = (b < 4) ? CODEC_BRIGHT : CODEC_DIM;
    tft.fillRect(portraitX + PORTRAIT_W + 10, portraitY + PORTRAIT_H - 12 - (b * 10), barW, 6, col);
  }
  // Microphone button on the left of the portrait
  micButton.visible = true;
  drawMicButton();

  if (chatState == CHAT_PENDING) {
    drawDialogueFrame();  // dots animation resumes from the main loop
  } else {
    drawAgentDialogue(lastDialogue);
  }
}

// ==========================================
// 5b. COUNTDOWN SCREEN (rocket + umbrella + timer)
// ==========================================

// ---- Starfield: stars streak downward so the rocket looks like it is flying up through space ----
#define NUM_STARS 28
#define STAR_MIN_X 16
#define STAR_MAX_X 223
#define STAR_MIN_Y 137
#define STAR_MAX_Y 258
#define STAR_INTERVAL 45

int16_t starX[NUM_STARS];
int16_t starY[NUM_STARS];
uint8_t starSpeed[NUM_STARS];  // 1..4 px per frame; faster stars are brighter and longer (parallax)
unsigned long lastStarTime = 0;

// Rough silhouette of the rocket (nose/body/nozzle/flame, plus the fin area); stars never draw here
bool inRocketMask(int x, int y) {
  const int cx = 120;
  if (abs(x - cx) <= 25 && y >= 136 && y <= 258) return true;
  if (abs(x - cx) <= 42 && y >= 192 && y <= 246) return true;
  return false;
}

void respawnStar(int i, bool anywhere) {
  starX[i] = random(STAR_MIN_X, STAR_MAX_X + 1);
  starY[i] = anywhere ? random(STAR_MIN_Y, STAR_MAX_Y + 1) : (STAR_MIN_Y - 1);
  starSpeed[i] = random(1, 5);
}

void drawStar(int i, bool erase) {
  int len = starSpeed[i];
  uint16_t col = erase ? TFT_BLACK : (len >= 3 ? CODEC_BRIGHT : (len == 2 ? CODEC_GREEN : CODEC_DIM));
  for (int k = 0; k < len; k++) {
    int py = starY[i] - k;  // head at the bottom, tail trailing above
    if (py < STAR_MIN_Y || py > STAR_MAX_Y) continue;
    if (!erase && (py & 1)) continue;  // even rows only, so the CRT scanlines stay intact
    if (inRocketMask(starX[i], py)) continue;
    tft.drawPixel(starX[i], py, col);
  }
}

void updateStarfield() {
  if (currentScreen != SCREEN_COUNTDOWN) return;
  if (millis() - lastStarTime < STAR_INTERVAL) return;
  lastStarTime = millis();
  for (int i = 0; i < NUM_STARS; i++) {
    drawStar(i, true);
    starY[i] += starSpeed[i];
    if (starY[i] - starSpeed[i] + 1 > STAR_MAX_Y) respawnStar(i, false);
    drawStar(i, false);
  }
}

// Open umbrella inspired by the Travelers logo: wide red dome, scalloped lower edge,
// curved panel ribs, pointed tip, and a shaft ending in a J-shaped handle
void drawUmbrella(int cx, int cy) {
  const int rx = 17;  // canopy half-width
  const int ry = 12;  // canopy height

  // Canopy: upper half-ellipse
  for (int dy = 0; dy <= ry; dy++) {
    float f = (float)dy / ry;
    int w = (int)round(rx * sqrt(1.0 - f * f));
    tft.drawFastHLine(cx - w, cy - dy, 2 * w + 1, COLOR_UMBRELLA_RED);
  }

  // Scalloped lower edge: three shallow rounded scallops hanging below the canopy line
  for (int s = -1; s <= 1; s++) {
    int sx = cx + s * 11;
    for (int dy = 1; dy <= 3; dy++) {
      float f = (dy - 0.5) / 3.0;
      int w = (int)round(5.5 * sqrt(1.0 - f * f));
      tft.drawFastHLine(sx - w, cy + dy, 2 * w + 1, COLOR_UMBRELLA_RED);
    }
  }

  // Gently curved ribs from the tip down to the notches between scallops
  for (int side = -1; side <= 1; side += 2) {
    float x0 = cx, y0 = cy - ry;
    float x1 = cx + side * 9, y1 = cy - ry * 0.45;  // control point bulges outward
    float x2 = cx + side * 5.5, y2 = cy + 2;
    for (int step = 0; step <= 24; step++) {
      float t = step / 24.0;
      float u = 1.0 - t;
      int px = (int)round(u * u * x0 + 2 * u * t * x1 + t * t * x2);
      int py = (int)round(u * u * y0 + 2 * u * t * y1 + t * t * y2);
      tft.drawPixel(px, py, TFT_BLACK);
    }
  }

  // Pointed tip
  tft.fillTriangle(cx, cy - ry - 5, cx - 2, cy - ry + 1, cx + 2, cy - ry + 1, COLOR_UMBRELLA_RED);

  // Shaft (3 px wide)
  tft.fillRect(cx - 1, cy + 3, 3, 15, COLOR_UMBRELLA_RED);

  // J-shaped handle curling to the left
  int hx = cx - 4;
  int hy = cy + 17;
  for (int r = 3; r <= 5; r++) {
    tft.drawCircleHelper(hx, hy, r, 0x0C, COLOR_UMBRELLA_RED);
  }
  tft.fillRect(cx - 9, hy - 4, 3, 5, COLOR_UMBRELLA_RED);
}

void drawRocket() {
  const int cx = 120;

  // Frame (same double border as the agent portrait)
  tft.drawRect(ROCKET_FRAME_X, ROCKET_FRAME_Y, ROCKET_FRAME_W, ROCKET_FRAME_H, CODEC_GREEN);
  tft.drawRect(ROCKET_FRAME_X + 2, ROCKET_FRAME_Y + 2, ROCKET_FRAME_W - 4, ROCKET_FRAME_H - 4, CODEC_DIM);

  // Swept fins (drawn first so the body overlaps their roots)
  for (int side = -1; side <= 1; side += 2) {
    int ax = cx + side * 22;
    int bx = cx + side * 40;
    tft.fillTriangle(ax, 196, bx, 244, ax, 230, CODEC_GREEN);
    tft.drawLine(ax, 196, bx, 244, CODEC_BRIGHT);
    tft.drawLine(bx, 244, ax, 230, CODEC_BRIGHT);
    tft.drawLine(ax, 208, cx + side * 35, 236, CODEC_DIM);
  }

  // Engine nozzle
  tft.fillTriangle(cx - 11, 232, cx + 11, 232, cx + 15, 244, CODEC_DIM);
  tft.fillTriangle(cx - 11, 232, cx + 15, 244, cx - 15, 244, CODEC_DIM);
  tft.drawLine(cx - 11, 232, cx - 15, 244, CODEC_GREEN);
  tft.drawLine(cx + 11, 232, cx + 15, 244, CODEC_GREEN);
  tft.drawFastHLine(cx - 15, 244, 31, CODEC_GREEN);

  // Body
  tft.fillRect(cx - 22, 172, 45, 60, CODEC_DIM);

  // Nose cone: filled ogive with a highlight streak
  int prevHw = 0;
  for (int y = 138; y <= 172; y++) {
    float t = (y - 138) / 34.0;
    int hw = (int)round(22.0 * pow(t, 0.75));
    tft.drawFastHLine(cx - hw, y, 2 * hw + 1, CODEC_GREEN);
    if (y > 138) {
      tft.drawLine(cx - prevHw, y - 1, cx - hw, y, CODEC_BRIGHT);
      tft.drawLine(cx + prevHw, y - 1, cx + hw, y, CODEC_BRIGHT);
    }
    if (y > 146) tft.drawPixel(cx - (hw * 55) / 100, y, CODEC_BRIGHT);
    prevHw = hw;
  }

  // Porthole in the nose cone
  tft.drawCircle(cx, 162, 5, CODEC_BRIGHT);
  tft.fillCircle(cx, 162, 4, TFT_BLACK);
  tft.drawCircle(cx, 162, 2, CODEC_DIM);
  tft.drawPixel(cx - 2, 160, CODEC_BRIGHT);
  tft.drawPixel(cx - 1, 159, CODEC_BRIGHT);

  // Body outline, collar band, and cylinder shading
  tft.drawFastVLine(cx - 22, 172, 60, CODEC_BRIGHT);
  tft.drawFastVLine(cx + 22, 172, 60, CODEC_BRIGHT);
  tft.fillRect(cx - 22, 172, 45, 4, CODEC_GREEN);
  tft.drawFastHLine(cx - 22, 175, 45, CODEC_BRIGHT);
  tft.drawFastHLine(cx - 22, 232, 45, CODEC_BRIGHT);
  tft.drawFastVLine(cx - 18, 178, 50, CODEC_GREEN);
  tft.drawFastVLine(cx + 17, 178, 50, TFT_BLACK);
  tft.drawFastVLine(cx + 18, 178, 50, TFT_BLACK);

  // CRT scanlines over the whole frame, then the umbrella on top so it stays solid red
  applyScanlines(ROCKET_FRAME_X + 1, ROCKET_FRAME_Y + 1, ROCKET_FRAME_W - 2, ROCKET_FRAME_H - 2);
  drawUmbrella(cx, 201);
}

// Flickering engine flame (green only; the umbrella is the only red)
void updateRocketFlame() {
  if (currentScreen != SCREEN_COUNTDOWN) return;
  if (millis() - lastFlameTime < 110) return;
  lastFlameTime = millis();

  tft.fillRect(FLAME_X, FLAME_Y, FLAME_W, FLAME_H, TFT_BLACK);
  int cx = FLAME_X + FLAME_W / 2;
  int len = random(7, 12);
  for (int k = -2; k <= 2; k++) {
    int l = len - abs(k) * 2 + random(-1, 2);
    if (l < 2) l = 2;
    tft.drawFastVLine(cx + k * 4, FLAME_Y, l, (k == 0) ? CODEC_BRIGHT : CODEC_GREEN);
  }
  applyScanlines(FLAME_X, FLAME_Y, FLAME_W, FLAME_H);
}

int16_t countdownBoxX(int i) {
  int16_t x0 = (tft.width() - (4 * CD_BOX_W + 3 * CD_BOX_GAP)) / 2;
  return x0 + i * (CD_BOX_W + CD_BOX_GAP);
}

void drawCountdownBoxes() {
  static const char* labels[4] = { "DAYS", "HRS", "MIN", "SEC" };
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(CODEC_GREEN, TFT_BLACK);
  for (int i = 0; i < 4; i++) {
    int16_t x = countdownBoxX(i);
    tft.fillRect(x, CD_BOX_Y, CD_BOX_W, CD_BOX_H, TFT_BLACK);
    tft.drawRect(x, CD_BOX_Y, CD_BOX_W, CD_BOX_H, CODEC_GREEN);
    tft.drawRect(x + 2, CD_BOX_Y + 2, CD_BOX_W - 4, CD_BOX_H - 4, CODEC_DIM);
    tft.drawString(labels[i], x + CD_BOX_W / 2, CD_BOX_Y + 38, 1);
  }
}

// Redraws just one number (padding overwrites the old digits without flicker)
void drawCountdownValue(int i, long val) {
  char buf[8];
  if (val < 0) {
    strcpy(buf, "--");
  } else {
    snprintf(buf, sizeof(buf), "%02ld", val);
  }
  tft.setTextColor(CODEC_BRIGHT, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextPadding(CD_BOX_W - 8);
  tft.drawString(buf, countdownBoxX(i) + CD_BOX_W / 2, CD_BOX_Y + 18, 4);
  tft.setTextPadding(0);
}

void updateCountdown() {
  if (currentScreen != SCREEN_COUNTDOWN) return;
  if (millis() - lastCdCheck < 250) return;
  lastCdCheck = millis();

  long vals[4];
  long now = (long)time(nullptr);
  if (now < 1700000000L) {  // clock not synced via NTP yet
    for (int i = 0; i < 4; i++) vals[i] = -1;
  } else {
    long remaining = (long)COUNTDOWN_TARGET_EPOCH - now;
    if (remaining < 0) remaining = 0;
    vals[0] = remaining / 86400L;
    vals[1] = (remaining % 86400L) / 3600L;
    vals[2] = (remaining % 3600L) / 60L;
    vals[3] = remaining % 60L;
  }
  for (int i = 0; i < 4; i++) {
    if (vals[i] != lastCdValues[i]) {
      lastCdValues[i] = vals[i];
      drawCountdownValue(i, vals[i]);
    }
  }
}

void drawCountdownScreen() {
  for (int i = 0; i < NUM_HOME_BUTTONS; i++) {
    homeButtons[i].visible = false;
  }
  micButton.visible = false;
  backButton.visible = true;
  drawCodecButton(backButton);

  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(CODEC_BRIGHT, TFT_BLACK);
  tft.setTextDatum(TR_DATUM);
  tft.drawString("T-MINUS", tft.width() - 14, 66);

  tft.setTextDatum(TC_DATUM);
  tft.drawString("Assisted Voice Auto", tft.width() / 2, 94);
  tft.drawString("Production Release", tft.width() / 2, 112);

  drawRocket();
  drawCountdownBoxes();

  for (int i = 0; i < 4; i++) lastCdValues[i] = -2;  // force first paint
  for (int i = 0; i < NUM_STARS; i++) respawnStar(i, true);
  lastStarTime = 0;
  lastCdCheck = 0;
  lastFlameTime = 0;
}

void navigateTo(ScreenID nextScreen) {
  currentScreen = nextScreen;
  tft.fillRect(0, HEADER_HEIGHT, tft.width(), tft.height() - HEADER_HEIGHT, TFT_BLACK);
  switch (currentScreen) {
    case SCREEN_HOME:
      drawHomeScreen();
      break;
    case SCREEN_AI_AGENT:
      drawAIAgentScreen();
      break;
    case SCREEN_COUNTDOWN:
      drawCountdownScreen();
      break;
  }
}

// ==========================================
// 6. CHAT REQUEST (background task + response parsing)
// ==========================================

// Map a Unicode codepoint to something the built-in ASCII TFT font can show
String foldCodepoint(uint32_t cp) {
  if (cp >= 32 && cp < 127) return String((char)cp);
  switch (cp) {
    case 0x00A0: return " ";
    case 0x2018:
    case 0x2019: return "'";
    case 0x201C:
    case 0x201D: return "\"";
    case 0x2013:
    case 0x2014: return "-";
    case 0x2026: return "...";
    default: return "";  // drop anything else (emoji, accents, etc.)
  }
}

// Minimal parser for {"reply": "..."}: handles JSON escapes and UTF-8 so no JSON library is needed
String extractReply(const String& body) {
  int k = body.indexOf("\"reply\"");
  if (k < 0) return "";
  k = body.indexOf(':', k);
  if (k < 0) return "";
  k = body.indexOf('"', k);
  if (k < 0) return "";

  String out;
  int len = body.length();
  for (int i = k + 1; i < len; i++) {
    char c = body[i];
    if (c == '"') break;
    if (c == '\\' && i + 1 < len) {
      char n = body[++i];
      switch (n) {
        case 'n': out += '\n'; break;
        case 't': out += ' '; break;
        case 'u': {
          uint32_t cp = strtoul(body.substring(i + 1, i + 5).c_str(), nullptr, 16);
          i += 4;
          out += foldCodepoint(cp);
          break;
        }
        default: out += n; break;  // \" \\ \/
      }
    } else if ((uint8_t)c >= 0x80) {
      uint8_t b = (uint8_t)c;
      int extra = (b >= 0xF0) ? 3 : (b >= 0xE0) ? 2 : (b >= 0xC0) ? 1 : 0;
      uint32_t cp = (extra == 3) ? (b & 0x07) : (extra == 2) ? (b & 0x0F) : (extra == 1) ? (b & 0x1F) : 0;
      for (int j = 0; j < extra && i + 1 < len; j++) {
        cp = (cp << 6) | ((uint8_t)body[++i] & 0x3F);
      }
      out += foldCodepoint(cp);
    } else {
      out += c;
    }
  }
  out.trim();
  return out;
}

// Runs on core 0 so the UI loop (core 1) keeps animating while we wait
void chatTaskFn(void* param) {
  String result;
  bool ok = false;

  if (WiFi.status() != WL_CONNECTED) {
    result = "NO WIFI LINK";
  } else {
    HTTPClient http;
    http.setConnectTimeout(5000);
    http.setTimeout(CHAT_TIMEOUT_MS);
    if (http.begin(CHAT_URL)) {
      http.addHeader("Content-Type", "application/json");
      int code = http.POST("{}");  // empty body -> server uses its hardcoded prompt
      if (code == 200) {
        result = extractReply(http.getString());
        ok = result.length() > 0;
        if (!ok) result = "EMPTY RESPONSE";
      } else if (code > 0) {
        result = "SERVER ERROR " + String(code);
      } else {
        result = "LINK FAILED:\n" + http.errorToString(code);
      }
      http.end();
    } else {
      result = "BAD URL";
    }
  }

  chatResult = result;
  chatState = ok ? CHAT_DONE : CHAT_ERROR;  // flip last so the UI sees a complete result
  vTaskDelete(nullptr);
}

void startChatRequest() {
  if (chatState != CHAT_IDLE) return;
  chatState = CHAT_PENDING;
  dotsPhase = 0.0;
  lastDotsTime = 0;
  drawMicButton();      // dims the icon
  drawDialogueFrame();  // clears the box; dots animate from loop()
  xTaskCreatePinnedToCore(chatTaskFn, "chat", 10240, nullptr, 1, nullptr, 0);
}

void updateChatRequest() {
  if (chatState == CHAT_PENDING) {
    if (currentScreen == SCREEN_AI_AGENT && millis() - lastDotsTime >= DOTS_INTERVAL) {
      lastDotsTime = millis();
      drawBouncingDots();
      dotsPhase += 0.28;
    }
  } else if (chatState == CHAT_DONE || chatState == CHAT_ERROR) {
    lastDialogue = chatResult;
    chatState = CHAT_IDLE;
    if (currentScreen == SCREEN_AI_AGENT) {
      drawAgentDialogue(lastDialogue);
      drawMicButton();  // restore bright icon
    }
  }
}

// ==========================================
// 7. TOUCH & SERIAL INPUT HANDLER
// ==========================================
void handleInputs() {
  uint16_t tx = 0, ty = 0;
  if (tft.getTouch(&tx, &ty)) {
    if (millis() - lastTouchTime >= TOUCH_DEBOUNCE) {
      lastTouchTime = millis();
#if TOUCH_DEBUG
      Serial.printf("touch x=%u y=%u\n", tx, ty);
#endif
      // Pass 1 tries exact hits; pass 2 falls back to the legacy mirrored hit test.
      // (Mirroring can't tell two buttons apart, so exact hits must win.)
      if (currentScreen == SCREEN_HOME) {
        for (int i = 0; i < NUM_HOME_BUTTONS; i++) {
          if (homeButtons[i].containsDirect(tx, ty)) {
            navigateTo(homeButtons[i].targetScreen);
            return;
          }
        }
        for (int i = 0; i < NUM_HOME_BUTTONS; i++) {
          if (homeButtons[i].contains(tx, ty, tft.width(), tft.height())) {
            navigateTo(homeButtons[i].targetScreen);
            return;
          }
        }
      } else {
        if (backButton.containsDirect(tx, ty)) {
          navigateTo(backButton.targetScreen);
          return;
        }
        if (micButton.containsDirect(tx, ty)) {
          startChatRequest();
          return;
        }
        if (backButton.contains(tx, ty, tft.width(), tft.height())) {
          navigateTo(backButton.targetScreen);
          return;
        }
        if (micButton.contains(tx, ty, tft.width(), tft.height())) {
          startChatRequest();
          return;
        }
      }
    }
  }
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'b' || cmd == 'B') {
      navigateTo(SCREEN_HOME);
    } else if (cmd == '1' && currentScreen == SCREEN_HOME) {
      navigateTo(SCREEN_AI_AGENT);
    } else if (cmd == '2' && currentScreen == SCREEN_HOME) {
      navigateTo(SCREEN_COUNTDOWN);
    }
  }
}

// ==========================================
// 8. SETUP & MAIN LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  randomSeed(esp_random());
  tft.init();
  tft.setRotation(0);  // Portrait (240x320)
  tft.fillScreen(TFT_BLACK);
  logoX = (tft.width() - SCHUNET_LOGO_WIDTH) / 2;
  logoY = 10;
  drawStaticHeader();
  globeSprite.createSprite(GLOBE_SIZE, GLOBE_SIZE);
  eyeSprite.createSprite(EYE_BOX_W, EYE_BOX_H);
  dotsSprite.createSprite(DOTS_W, DOTS_H);
  navigateTo(SCREEN_HOME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  // 1. Top Wi-Fi Status Globe Animation
  bool isConnected = (WiFi.status() == WL_CONNECTED);
  if (isConnected && !ntpStarted) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // UTC; countdown target is a UTC epoch
    ntpStarted = true;
  }
  if (isConnected) {
    wasConnected = true;
    if (millis() - lastAnimTime >= ANIM_INTERVAL) {
      lastAnimTime = millis();
      drawSpinningGlobe(globePhase);
      globePhase += 0.06;
      if (globePhase >= PI) globePhase -= PI;
    }
  } else if (wasConnected) {
    wasConnected = false;
    globePhase = 0.0;
    drawStaticHeader();
  }

  // 2. AI Agent Eye Blink & Gaze Animation
  updateAgentEyes();

  // 3. Chat request progress (bouncing dots / show reply)
  updateChatRequest();

  // 4. Countdown screen (timer digits + rocket flame)
  updateCountdown();
  updateRocketFlame();
  updateStarfield();

  // 5. Touch & Serial Navigation
  handleInputs();
}

#include <TFT_eSPI.h>
#include <SPI.h>
#include "touch.h"

TFT_eSPI my_lcd = TFT_eSPI();

void setup() {
  Serial.begin(115200);

  my_lcd.init();
  my_lcd.setRotation(0);
  touch_init(my_lcd.width(), my_lcd.height(), my_lcd.getRotation());

  my_lcd.fillScreen(TFT_BLACK);

  // Header
  my_lcd.fillRect(0, 0, my_lcd.width(), 40, TFT_NAVY);
  my_lcd.setTextColor(TFT_CYAN);
  my_lcd.drawString("Touch Coordinates", 20, 12, 2);

  // Prompt
  my_lcd.setTextColor(TFT_DARKGREY);
  my_lcd.drawString("Touch anywhere...", 30, my_lcd.height()/2, 2);

  Serial.println("Touch test ready");
}

void loop() {
  if (touch_touched()) {
    int x = touch_last_x;
    int y = touch_last_y;

    Serial.print("X: "); Serial.print(x);
    Serial.print("  Y: "); Serial.println(y);

    // Clear screen (keep header)
    my_lcd.fillRect(0, 41, my_lcd.width(), my_lcd.height() - 41, TFT_BLACK);

    // ── Coordinates box ──────────────────────────────
    my_lcd.fillRoundRect(20, 60, my_lcd.width() - 40, 80, 10, TFT_NAVY);
    my_lcd.drawRoundRect(20, 60, my_lcd.width() - 40, 80, 10, TFT_CYAN);

    my_lcd.setTextColor(TFT_LIGHTGREY);
    my_lcd.drawString("X:", 35, 75, 2);
    my_lcd.drawString("Y:", 35, 105, 2);

    my_lcd.setTextColor(TFT_YELLOW);
    my_lcd.drawString(String(x), 65, 75, 2);
    my_lcd.drawString(String(y), 65, 105, 2);

    // ── Crosshair at touch point ──────────────────────
    // Clamp to safe draw area (below header)
    int drawY = max(y, 45);

    // Clear previous crosshair area
    // Horizontal line
    my_lcd.drawFastHLine(0, drawY, my_lcd.width(), TFT_DARKGREY);
    // Vertical line
    my_lcd.drawFastVLine(x, 41, my_lcd.height() - 41, TFT_DARKGREY);

    // Dot at touch point
    my_lcd.fillCircle(x, drawY, 6, TFT_RED);
    my_lcd.drawCircle(x, drawY, 6, TFT_WHITE);

    // ── Raw values (smaller, bottom) ──────────────────
    my_lcd.setTextColor(TFT_DARKGREY);
    my_lcd.drawString("raw x=" + String(x) + " y=" + String(y), 10, my_lcd.height() - 20, 1);

    // Wait for release
    while (touch_touched());
  }
}
#include <TFT_eSPI.h>

#define RX_PIN 18
#define TX_PIN 17

HardwareSerial rfidSerial(1); // UART1
TFT_eSPI tft = TFT_eSPI();

String lastCard = "";
unsigned long lastScanTime = 0;
const unsigned long DEBOUNCE_MS = 1500;
int scanCount = 0;

// ── Waiting screen ───────────────────────────────────────────
void showWaiting() {
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 320, 45, 0x1082);
  tft.setTextColor(TFT_CYAN, 0x1082);
  tft.setTextSize(2);
  tft.setCursor(75, 12);
  tft.println("RFID SCANNER");

  tft.drawRoundRect(110, 65, 100, 70, 10, TFT_DARKGREY);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(133, 80);
  tft.println("[]");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(55, 155);
  tft.println("Scan a card...");

  if (scanCount > 0) {
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(10, 225);
    tft.print("Total scans: ");
    tft.println(scanCount);
  }
}

// ── Card detected screen ─────────────────────────────────────
void showCard(String hexUID, unsigned long decUID) {
  tft.fillScreen(TFT_BLACK);

  tft.fillRect(0, 0, 320, 45, TFT_DARKGREEN);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  tft.setTextSize(2);
  tft.setCursor(75, 12);
  tft.println("CARD DETECTED");

  // HEX UID
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 65);
  tft.println("HEX UID:");
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(3);
  int16_t x = (320 - (hexUID.length() * 18)) / 2;
  tft.setCursor(x > 0 ? x : 10, 95);
  tft.println(hexUID);

  tft.drawFastHLine(10, 140, 300, TFT_DARKGREY);

  // DEC UID
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 150);
  tft.println("DEC UID:");
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(20, 178);
  tft.println(String(decUID));

  tft.drawFastHLine(10, 210, 300, TFT_DARKGREY);

  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 220);
  tft.print("Scan #");
  tft.println(scanCount);
}

void setup() {
  Serial.begin(115200);
  rfidSerial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  pinMode(42, OUTPUT);
  digitalWrite(42, HIGH);

  tft.init();
  tft.setRotation(1);

  showWaiting();
  Serial.println("Raw monitor + parser ready");
}

void loop() {
  static byte buffer[12]; // 02 + 8 UID + 0D + 0A + 03 = 12 bytes
  static byte idx = 0;

  while (rfidSerial.available()) {
    byte b = rfidSerial.read();

    // Raw hex debug
    Serial.print("0x");
    if (b < 0x10) Serial.print("0");
    Serial.print(b, HEX);
    Serial.print(" ");

    // State machine
    if (b == 0x02) {
      idx = 0;
      buffer[idx++] = b;
    } else if (idx > 0 && idx < 12) {
      buffer[idx++] = b;

      if (idx == 12) {
        // Validate frame: STX=0x02, ETX=0x03, CR=0x0D at [9], LF=0x0A at [10]
        if (buffer[0] == 0x02 && buffer[9] == 0x0D &&
            buffer[10] == 0x0A && buffer[11] == 0x03) {

          // Extract 8-char UID (bytes 1–8)
          char hexUID[9];
          for (int i = 0; i < 8; i++) hexUID[i] = buffer[i + 1];
          hexUID[8] = '\0';

          unsigned long decUID = strtoul(hexUID, NULL, 16);
          String uidStr = String(hexUID);
          unsigned long now = millis();

          Serial.print("\n>>> Valid tag!  HEX: ");
          Serial.print(hexUID);
          Serial.print("  DEC: ");
          Serial.println(decUID);

          // Debounce
          if (uidStr != lastCard || (now - lastScanTime) > DEBOUNCE_MS) {
            scanCount++;
            lastCard = uidStr;
            lastScanTime = now;

            showCard(uidStr, decUID);
            delay(4000);

            // Flush stale bytes then go back to waiting
            while (rfidSerial.available()) rfidSerial.read();
            showWaiting();
          }

        } else {
          Serial.print("\n>>> Bad frame: ");
          for (int i = 0; i < 12; i++) {
            Serial.print(buffer[i], HEX);
            Serial.print(" ");
          }
          Serial.println();
        }
        idx = 0; // reset for next packet
      }
    }
  }
}
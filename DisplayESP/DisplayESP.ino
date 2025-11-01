#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include "driver/twai.h"

// ---- TTGO T-Display pins ----
#define TFT_DC    16
#define TFT_RST   23
#define TFT_CS     5      // set to -1 if your board ties CS low
#define TFT_MOSI  19
#define TFT_SCLK  18
#define TFT_BL     4

#if (TFT_CS >= 0)
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
#else
Adafruit_ST7789 tft(TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
#endif

// ---- CAN (TWAI) ----
static const gpio_num_t CAN_TX = GPIO_NUM_22;  // ESP32 → transceiver TXD
static const gpio_num_t CAN_RX = GPIO_NUM_21;  // ESP32 ← transceiver RXD
static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

// ---- UI ----
static const int LINE_H = 12;       // row height in pixels (font 1)
static const int HEADER_LINES = 2;  // rows used by header
int rowsFit = 0;                    // computed at runtime
int nextRow = 0;                    // 0..rowsFit-1

String fmtFrame(const twai_message_t &rx) {
  char buf[64];
  String s;

  if (rx.extd) snprintf(buf, sizeof(buf), "EXT %08lX ", (unsigned long)rx.identifier);
  else         snprintf(buf, sizeof(buf), "STD %03lX ", (unsigned long)rx.identifier);
  s += buf;

  snprintf(buf, sizeof(buf), "DLC=%d ", rx.data_length_code);
  s += buf;

  if (rx.rtr) s += "RTR";
  else {
    s += "DATA=";
    for (int i = 0; i < rx.data_length_code; ++i) {
      snprintf(buf, sizeof(buf), "%02X", rx.data[i]);
      s += buf;
      if (i + 1 < rx.data_length_code) s += ' ';
    }
  }
  return s;
}

void setupDisplay() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init(135, 240);
  tft.setRotation(3);     // landscape
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft.setTextWrap(false);

  // header
  tft.setCursor(0, 0);
  tft.println("CAN RX @500k");
  tft.println("ID   DLC  DATA");

  // compute rows we can show below header
  rowsFit = (tft.height() / LINE_H) - HEADER_LINES;
  if (rowsFit < 1) rowsFit = 1;
  nextRow = 0;
}

void printRow(const String &s) {
  // y position for the row we’re overwriting
  int y = (HEADER_LINES + nextRow) * LINE_H;

  // clear just this row, then print
  tft.fillRect(0, y, tft.width(), LINE_H, ST77XX_BLACK);
  tft.setCursor(0, y);
  tft.print(s);

  // advance (wrap when hitting bottom)
  nextRow++;
  if (nextRow >= rowsFit) nextRow = 0;
}

void setupCAN() {
  twai_general_config_t g_config =
      TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX, CAN_RX, TWAI_MODE_LISTEN_ONLY);
  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  ESP_ERROR_CHECK(twai_start());
}

void setup() {
  Serial.begin(115200);
  delay(200);
  setupDisplay();
  setupCAN();
}

void loop() {
  twai_message_t rx;
  if (twai_receive(&rx, 0) == ESP_OK) {
    String line = fmtFrame(rx);
    Serial.println(line);
    printRow(line);  // fast partial redraw
  }
}

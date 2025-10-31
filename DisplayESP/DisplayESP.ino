#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "driver/twai.h"

TFT_eSPI tft;

// --- CAN pins ---
static const gpio_num_t CAN_TX = GPIO_NUM_22;
static const gpio_num_t CAN_RX = GPIO_NUM_21;

static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

// --- Display settings ---
static const int LINE_H = 12;
static const int MAX_ROWS = 20;   // how many lines to keep visible
String lines[MAX_ROWS];
int lineCount = 0;

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

void redraw() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  for (int i = 0; i < lineCount; i++) {
    tft.println(lines[i]);
  }
}

void addLine(const String &s) {
  if (lineCount < MAX_ROWS) {
    lines[lineCount++] = s;
  } else {
    // shift lines up
    for (int i = 1; i < MAX_ROWS; i++) lines[i - 1] = lines[i];
    lines[MAX_ROWS - 1] = s;
  }
  redraw();
}

void setupDisplay() {
  tft.init();
  tft.setRotation(1); // landscape
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(1);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextWrap(false, false);

  addLine("TTGO T-Display CAN RX @500k");
  addLine("Listening...");
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
    addLine(line);
    Serial.println(line);
  }
  delay(2);
}

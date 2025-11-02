# CAN Breadboard (ESP32 / TWAI)

![Breadboard setup](BreadboardPhoto.JPG)

Small breadboard experiments for CAN bus using ESP32 boards and the built‑in TWAI (CAN) driver. One sketch transmits a heartbeat frame once per second; the other listens and shows received frames on a small TFT display (TTGO T‑Display).

## Repository Contents
- `SuperMiniESP32/SuperMiniESP32.ino` — ESP32‑S3 example that transmits a CAN frame every second and also prints any received frames to `Serial`.
- `DisplayESP/DisplayESP.ino` — ESP32 with TTGO T‑Display that listens on CAN and renders frames on the display.

## Hardware
- CAN transceiver (e.g., SN65HVD230, TJA1050, MCP2551 or similar) between ESP32 and the bus.
- Proper bus termination: 120 Ω at both ends of the CAN network.
- 3.3 V logic on ESP32; ensure your transceiver is 3.3 V‑compatible.

### Wiring (SuperMiniESP32)
- ESP32‑S3 `GPIO12` → Transceiver `TXD` (controller TX)
- ESP32‑S3 `GPIO13` ← Transceiver `RXD` (controller RX)
- CANH/CANL from the transceiver to the CAN bus

Baud rate: 500 kbit/s (configured in code). Adjust pins/bitrate for your board if needed.

### Wiring (DisplayESP with TTGO T‑Display)
- CAN pins: `GPIO22` (TX) → Transceiver `TXD`, `GPIO21` (RX) ← Transceiver `RXD`.
- Sketch uses listen‑only mode to monitor the bus.

## Software & Libraries
- Arduino core for ESP32 (Espressif) v2.0+ so `driver/twai.h` is available.
- For `DisplayESP` only:
  - `Adafruit_GFX`
  - `Adafruit_ST7789`

Install libraries via Arduino Library Manager or your preferred environment.

I tried using Bodmer's Screen Driver - But I couldn't get the output that I wanted. 

## Sketch Behavior
### SuperMiniESP32
- Initializes TWAI at 500 kbit/s and starts in normal mode.
- Every 1 s sends a standard (11‑bit) frame ID `0xABC` with an 8‑byte payload containing a counter and the bytes `S3CA`.
- Prints TX status and any received frames to the Serial Monitor at 115200 baud.

For the 4 different chips, I used 123, 456, 789 ABC etc. 

### DisplayESP
- Initializes the ST7789 (landscape) and shows a scrolling list of received frames.
- Configures TWAI in listen‑only mode at 500 kbit/s.
- For each received frame, prints a formatted line to the Serial Monitor and the TFT.

## Building & Uploading
1. Open each sketch folder (`SuperMiniESP32` or `DisplayESP`) in Arduino IDE or VS Code + Arduino extension.
2. Select an appropriate ESP32 board:
   - For `SuperMiniESP32`: an ESP32‑S3 variant (e.g., “ESP32S3 Dev Module”).
   - For `DisplayESP`: your TTGO T‑Display variant (or compatible ESP32 board) and ensure the TFT pin map matches your hardware.
3. Set Serial Monitor to 115200 baud.
4. Compile and upload.

## Notes
- If you only have one node on a bench setup, consider `TWAI_MODE_NO_ACK` for single‑node testing. The provided `DisplayESP` sketch uses `LISTEN_ONLY` to avoid interfering with the bus.
- If your bus uses a different bitrate, change the timing macro (e.g., `TWAI_TIMING_CONFIG_250KBITS()`).
- If your board uses different pins, update the `GPIO` numbers in each sketch accordingly.

---

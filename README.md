# ESP32-S3 Power Meter (INA3221 ·  TFT · Qt Ui · SQLlite data base)


An ESP32-S3–based **three-channel power meter** built around INA3221. One channel supports **USB-C pass-through** so you can measure power flowing from **USB-C IN → USB-C OUT**; **CC pins are tied** for basic PD trigger behavior (power-only path).

> Firmware stack: **FreeRTOS**.
> REST/WebSocket API is **optional** (may be added later).
> Live data can be streamed over **USB-CDC**; an **optional ST7735 (160×128) SPI TFT** can show basic status (no touch).
>
> A custom **3D-printed enclosure** is planned.

---

---

## Features

* **3 measurement channels** via **INA3221**

  * Voltage (V), Current (A), Power (W), accumulated **mAh / mWh**
* **ESP32-S3** (Wi‑Fi + native USB/CDC)

  * UART header and USB device available on the board
* **USB‑C pass‑through (1 channel)**: measure power across IN → OUT

  * **CC lines tied** (basic PD role trigger; data lines not routed)
* **Optional TFT:** **ST7735 (160×128) SPI** for basic on-device status (no touch)
* **Battery**: **TP4056** Li‑ion charger + **AMS1117‑3.3** LDO
* **Enclosure**: custom **3D‑printed** case (to be published)

---

## What’s New

* Qt UI with Python and SQLite database integration

* Added boot-time Check screen: verifies Wi‑Fi link and sensor health before entering the UI.

* Check screen now shows connection status and the assigned IP address.

* Main screen updated to display the device IP for quick access to the Web UI.

* **ST7735 (160×128) screen added (no touch).**

* Energy counter buttons are active (Start/Stop/Reset)

* Sensor → UART JSON and Web UI hooked up

* 4-task FreeRTOS skeleton: sensor, screen, web, uart

## Gallery

<p align="center">
  <img src="Photos/Check_Screen.jpg" width="40%">&nbsp;&nbsp;
  <img src="Photos/Main_Screen.jpg" width="40%"><br>
  <em> First Check Screen &nbsp; | &nbsp; Main Screen </em>
</p>

<p align="center">
  <img src="Photos/First_Run.jpg" width="40%">&nbsp;&nbsp;
  <img src="Photos/with_st7735.jpg" width="40%"><br>
  <em> First Run Without Screen &nbsp; | &nbsp; With ST7735</em>
</p>

<p align="center">
  <img src="Photos/PCB_Model.jpg" width="45%">&nbsp;&nbsp;
  <img src="Photos/PCB.jpg" width="45%"><br>
  <em>3D board render &nbsp; | &nbsp; PCB layout</em>
</p>

<p align="center">
  <!-- Add your prototype photos below when ready -->
  <img src="Photos/Prototype.jpg" width="100%">&nbsp;&nbsp;
  <em>Prototype (front) </em>
</p>

<p align="center">
  <img src="Photos/Real_PCB.jpg" width="60%"><br>
  <em>Producted PCB (first article)</em>
</p>

<p align="center">
  <img src="Photos/PCB_Assembled.jpg" width="60%"><br>
  <em>Assembled PCB </em>
</p>

<p align="center">
  <img src="Photos/Schematic.png" width="85%"><br>
  <em>Schematic (block view)</em>
</p>

<p align="center">
  <img src="Photos/Qt_Ui_Early.png" width="85%"><br>
  <em> Qt Ui </em>
</p>

## Hardware Overview

* **MCU:** ESP32‑S3 module
* **Power Monitor:** INA3221 (three shunts)
* **Optional Display:** ST7735 (160×128) over SPI (no touch)
* **Power Path:** USB‑C IN, USB‑C OUT (pass‑through on Channel 1)
* **PMICs:** TP4056 Li‑ion charger, AMS1117‑3.3 regulator
* **Connectors:** 3× screw terminals (IN/OUT/GND), UART header, 2× USB‑C

### Block Diagram

```
USB-C IN ──┐                  ┌── USB-C OUT (pass-through)
           ├─ Shunt CH1  ─────┤
IN/OUT CH2 ├─ Shunt CH2  ─────┤──> Loads
IN/OUT CH3 ├─ Shunt CH3  ─────┘
           │
        INA3221  ── I2C ── ESP32-S3 ── SPI ── ST7735 TFT (optional)
                               ├─ (Optional) Wi‑Fi HTTP/WS
                               └─ USB‑CDC / UART (streaming)
```

---

## Default Pins

| Function                       | ESP32‑S3 Pin      |
| ------------------------------ | ----------------- |
| I2C SDA / SCL (INA3221)        | GPIO __ / __      |
| TFT (ST7735) MOSI / MISO / SCK | GPIO __ / __ / __ |
| TFT CS / DC / RST              | GPIO __ / __ / __ |
| UART RX / TX                   | GPIO __ / __      |
| USB‑CDC                        | Native (S3)       |

---

## Firmware (planned)

* **Framework:** FreeRTOS
* **Modules:**

  * `sensor` – INA3221 sampling, filtering, mAh/mWh accumulation
  * `ui` *(optional)* – minimal status pages on **ST7735**; device can run headless
  * *(Optional)* `web` – HTTP + WebSocket for live JSON
  * `usb_stream` – ASCII/JSON streaming over USB‑CDC (no file logging)
* **Example JSON (USB/Web, if enabled):**

```json
{"ch":1,"voltage":5.08,"current":1.12,"power":5.69,"mah_total":23.4,"mwh_total":118.9,"ts":1730529652}
```

---

### Efficiency Measurement

The board can measure real conversion efficiency by comparing input and output power:

* Channels: CH1 = input (P_in), CH2 = output (P_out)
* Instantaneous: η_inst = P_out / P_in
* Averaged (stable): EMA over 1–3 s → η_avg =  P̄_out / P̄_in
* Energy-based: η_energy = ΔmWh_out / ΔmWh_in (robust against short spikes)


## Repo Layout (suggested)

```
/hardware
  schematic.pdf
  gerber/
/firmware
  /main
    board.h
    app_sensor.c
    app_ui_tft.c
    app_usb_stream.c
    (optional) app_web.c
  sdkconfig.defaults
/Photos
  PCB.jpg
  PCB_Model.jpg
  Prototype.jpg
  Prototype_Back1.jpg
  Prototype_Back2.jpg
  Real_PCB.jpg
  Schematic.png
/enclosure
  stl/  (3D-printed case files)
```

---

## Roadmap

* [ ] Bring‑up: INA3221 I2C readout
* [ ] Publish 3D‑printed enclosure (STL)
* [ ] USB‑CDC live streaming (no on‑device logs)
* [ ] Minimal on-device status (ST7735)
* [ ] Calibration UI + NVS storage
* [ ] (Optional) REST/WS endpoints

---

## Notes & Safety

* USB‑C pass‑through is **power‑only** (data lines not connected).
* CC lines support **basic PD trigger behavior**; PD profile support is **limited**.
* Verify shunt resistor wattage and temperature rise.
* Provide adequate thermal copper for **TP4056** during charging.

---

## License

MIT (to be added).

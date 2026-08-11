[README.md](https://github.com/user-attachments/files/30927621/README.md)
# ESP32-S3 USB Keyboard to BLE HID

A USB-to-Bluetooth keyboard adapter based on the ESP32-S3.

The project reads input from a wired USB HID keyboard using the ESP32-S3 native USB Host interface, processes the HID reports, and forwards the keyboard input as a BLE HID keyboard.

## Architecture

```text
USB Keyboard
     |
     | USB HID
     v
+----------------+
|   ESP32-S3     |
|                |
|   USB Host     |
|      |         |
|   HID Reports  |
|      |         |
|    BLE HID     |
+-------+--------+
        |
        | Bluetooth
        v
 PC / Laptop / Tablet
```

## Features

- USB HID keyboard host
- BLE HID keyboard
- Standard keyboard keys
- Modifier keys
- Function keys
- Fn-layer controls
- Volume knob
- Volume up/down
- Mute
- Brightness controls
- Raw HID report debugging
- Separate keyboard and media-control HID interface handling

## Hardware

### Controller

- ESP32-S3
- Tested with ESP32-S3 SuperMini / compatible ESP32-S3 boards
- Native USB Host
- Bluetooth Low Energy

### Tested Keyboard

**EvoFox Ronin TKL**

The keyboard exposes multiple HID interfaces:

| Interface | Protocol | Function |
|---|---:|---|
| Interface 0 | 1 | Boot keyboard |
| Interface 1 | 2 | Media / special controls |

### ESP32-S3 USB Pins

| Signal | ESP32-S3 |
|---|---|
| USB D- | GPIO19 |
| USB D+ | GPIO20 |

These are the native USB pins used by the ESP32-S3 USB peripheral.

## HID Reports

The keyboard interface produces standard 8-byte boot keyboard reports:

```text
Byte 0   Modifier
Byte 1   Reserved
Byte 2-7 Keycodes
```

Example:

```text
00 00 04 00 00 00 00 00
```

The firmware forwards the report to the BLE HID keyboard.

### Media / Special Controls

The second HID interface produces 3-byte reports.

Observed reports from the tested keyboard:

```text
03 E9 00  -> Volume Up
03 EA 00  -> Volume Down
03 E2 00  -> Mute / Knob Press
03 70 00  -> Brightness Up
03 6F 00  -> Brightness Down
```

The exact mappings may differ between keyboard models.

## Software

- ESP-IDF 5.5.1
- C
- FreeRTOS
- ESP32-S3 USB Host HID
- BLE HID

### Main Files

```text
main/
├── usb_host.c
├── usb_host.h
├── keyboard.c
├── keyboard.h
├── ble_hidd_demo_main.c
└── CMakeLists.txt
```

`usb_host.c` handles USB Host initialization, HID interface detection, input-report reception, and report forwarding.

`keyboard.c` handles BLE HID keyboard report transmission.

`ble_hidd_demo_main.c` contains the BLE HID application and connection handling.

## Build

Open an ESP-IDF terminal in the project directory:

```powershell
idf.py build
```

## Flash

```powershell
idf.py flash
```

Monitor:

```powershell
idf.py monitor
```

Or:

```powershell
idf.py flash monitor
```

## Debugging HID Reports

The firmware prints raw USB HID reports.

Keyboard example:

```text
INPUT REPORT -> Interface 0 Protocol 1
Report length: 8
RAW: 00 00 04 00 00 00 00 00
```

Media-control example:

```text
INPUT REPORT -> Interface 1 Protocol 2
Report length: 3
RAW: 03 E9 00
```

Raw report logging was used to identify the volume knob and Fn-layer controls.

## USB Debugging Limitation

The ESP32-S3 native USB peripheral is being used as the USB Host for the keyboard. Therefore, the same USB interface cannot simultaneously function as the normal PC USB serial/debug interface.

A separate UART connection can be used for debugging when required.

## Power

For a battery-powered version, the intended architecture is:

```text
Li-ion Battery
      |
      v
5V Boost / Power-bank Module <-----------------
      |                                        |
      |                                        |
      +----------> ESP32-S3 5V +----------> USB Keyboard 5V
```

The final hardware should use an appropriate power-path/load-sharing design so that USB 5 V sources do not directly fight each other.

## Testing

The prototype successfully handled:

- Alphabet keys
- Function keys
- Modifier keys
- Fn + function keys
- Brightness controls
- Mute
- Volume up
- Volume down
- Rotary volume knob
- Rotary knob press

## Project Status

**Working prototype**

The USB keyboard input is successfully converted to BLE HID keyboard input.

Possible future improvements:

- Battery monitoring
- Automatic power management
- BLE pairing button
- Connection-status LED
- Battery-level indicator
- Low-power/sleep operation
- Improved generic HID report parsing
- Support for more keyboard models
- Custom enclosure

## Important Hardware Note

The ESP32-S3 native USB interface uses:

```text
GPIO19 = USB D-
GPIO20 = USB D+
```

These USB signals cannot simply be moved to ordinary UART TX/RX pins.

If the USB D+ or D- physical connection is damaged, repair the physical USB path rather than attempting to remap USB to arbitrary GPIOs.

## License

Choose a license appropriate for your intended use. MIT is a common choice for open-source embedded projects.

## Author

**Anto**

ESP32-S3 • USB HID • BLE HID • Embedded Systems

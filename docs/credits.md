# Credits and licenses

| Source | Contribution |
|---|---|
| [Jamie Magee / specialized-turbo](https://github.com/JamieMagee/specialized-turbo), revision `4ca99f1a13e603e1d1e46dfe1aeb8bb6fc9eaa10` | Initial Bluetooth connection, control and telemetry experiments used to correlate CAN captures |
| [Bernd Woköck / LevoEsp32Ble](https://github.com/Sepp62/LevoEsp32Ble), revision `a843c2fbc0f327a9d8e9cda30c58ad01c9e57936` | Additional legacy BLE commands and field definitions |
| [ReadieFur / OpenTCU research](https://github.com/ReadieFur/OpenTCU/tree/research) | Earlier CAN and wake-circuit research used as context |
| [Waveshare ESP32-C6-Touch-LCD-1.47 demo](https://www.waveshare.com/wiki/ESP32-C6-Touch-LCD-1.47) | Board pin assignments, display/touch initialization and JD9853 driver |
| [Espressif ESP-IDF](https://github.com/espressif/esp-idf/tree/v5.4.2) | Build system, TWAI, GPIO, USB and display interfaces |
| [Roboto Condensed](https://github.com/googlefonts/roboto-classic) | Screen glyphs; source font and SIL OFL notice in `assets/fonts/` |

The BLE implementations and OpenTCU source are not bundled; their licenses apply to their respective projects. The CAN check-byte model, companion lookup, timing and field correlations here were checked against the project bike's captures. Credit does not imply endorsement or review by the authors above.

Original project code is MIT licensed. The JD9853 driver retains its Espressif Apache-2.0 notices; the font retains SIL OFL. Those notices govern their respective files. The bezel files were supplied by the builder. Code and documentation used AI assistance; [offline checks and physical testing](validation.md) are described separately.

Specialized and Turbo Levo identify the equipment. This is an independent project.

## Reproducing the font

`python3 tools/generate_clinical_font.py` generates `firmware/main/font.h` from bundled Roboto Condensed at weight 700. It requires Pillow. The 95 printable ASCII glyphs use 10×20 cells with a common baseline and bit 0 at the left. Retain the generated header for exact reproduction across font-renderer versions. This licensed font replaces the earlier bitmap whose source was not recorded.

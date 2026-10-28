# CAN protocol

This is the measured protocol used by the replacement controller on the builder’s [2021 Specialized Turbo Levo](https://www.specialized.com/us/en/turbo-levo/p/184430?color=293256-184430) with an SBC-D03 TCU. It includes enough of startup, assist control, Walk, telemetry, and shutdown to run that bike without its stock TCU. It is not a complete Specialized protocol specification or a compatibility claim for other bikes.

The portable [C codec](../protocol/levo_protocol.c), [Python codec](../protocol/levo.py), and [DBC](../protocol/levo-observed.dbc) can be used independently of the reference screen controller.

All CAN IDs and byte values below are hexadecimal. Offsets are zero-based. Multi-byte telemetry values are little-endian unless stated otherwise.

## Bus and evidence

- Classical CAN, 250 kbit/s, standard 11-bit IDs.
- Physically separating the stock TCU's CAN pair confirmed that it sends `0x100`, `0x300`, and `0x301`.
- Stock captures established the command encoding and timing. The replacement subsequently operated Off, Eco, Trail, Turbo, Walk, and shutdown on the test bike, on a workstand and on rides by the builder. These results belong to the fuller development firmware; the reference build in this repo has not been verified on hardware.
- Speed and cadence scales were compared with BLE readings during separate wheel-spin and pedaling tests. Load-to-torque scaling remains provisional.

The original capture analysis checked 66,551 command frames across 179 distinct payload/counter combinations and 12 payload states, with no encoder mismatch. It also checked 66,546 paired companion frames without mismatch. These counts describe the original private captures, not a test of every possible payload. The included [startup fixture](../tests/sequence-startup-fixture.txt) reproduces 852 command/companion frames for offline regression testing.

## Assist command: `0x300`, DLC 8

| Byte | Observed use |
|---|---|
| 0 | `00` for Off, `03` for enabled profiles; not an Eco/Trail/Turbo number |
| 1 | `5A` normally, `A5` during Walk |
| 2 | `00` during initial startup and shutdown, `64` while running |
| 3 | `5A` normally, `A5` during the final shutdown phase |
| 4 | Active support percentage, 0 to 100 decimal |
| 5 | `00` in the observed profiles; other uses unknown |
| 6 | Active peak-assist percentage; Off uses `0A` |
| 7 | Check nibble in bits 7..4, rolling counter in bits 3..0 |

Changing support and peak independently through BLE confirmed bytes 4 and 6. The other field names describe observed behavior, not a complete bit-level interpretation.

These are the running profiles captured on this bike:

| Profile | Bytes 0..6 | Support / peak, decimal |
|---|---|---|
| Off | `00 5A 64 5A 00 00 0A` | 0 / 10 |
| Eco | `03 5A 64 5A 23 00 23` | 35 / 35 |
| Trail | `03 5A 64 5A 32 00 46` | 50 / 70 |
| Turbo | `03 5A 64 5A 64 00 64` | 100 / 100 |
| Walk | `03 A5 64 5A 23 00 23` | 35 / 35 |

These are captured settings, not universal factory defaults. No separate CAN “assist up” or “assist down” command is required by this implementation. Buttons select a profile locally; the controller continues sending the selected absolute profile. Changing profiles does not reset the counter.

### Check nibble and counter

For payload bytes `b[0]` through `b[6]` and counter `n` from 0 to 15:

```python
check = b[0] & 0x0f
for i in range(1, 7):
    check ^= (b[i - 1] >> 4) | (b[i] & 0x0f)
check ^= (b[6] >> 4) | n
b[7] = ((check & 0x0f) << 4) | n
```

The adjacent nibbles are **ORed before XOR folding**. This empirical rule matches the observed frames; it is not a standard CRC or an extracted stock-firmware algorithm. The CAN controller separately handles the CAN frame's own CRC.

### Companion: `0x301`, DLC 3

Each command is followed by a companion with the same counter:

```text
byte 0 = table[n]
byte 1 = n
byte 2 = 00

table[0..15] = E2 B3 6B 96 03 54 F5 B8 80 88 C6 43 B5 4E 1D E8
```

The algorithm behind the table's first byte is unknown. The counter advances modulo 16 after each pair.

A complete Turbo pair at counter 0 is:

```text
ID 300  DLC 8  03 5A 64 5A 64 00 64 30
ID 301  DLC 3  E2 00 00
```

This is a valid encoded pair, not a standalone startup command. The bike also needs electrical wake, startup traffic, and a continuing command stream. Do not transmit a replacement stream alongside an active stock TCU.

## Startup and shutdown

The [sequencer](../protocol/tcu_sequence.c) schedules one command/companion pair every 50 ms. It places the companion 1 ms after the command. These are implementation timings based on stock captures, not measured limits of what the motor will accept.

Startup in the current controller:

1. Electrical wake brings up the bike. CAN traffic alone is not the implemented wake mechanism.
2. Wait for a valid `0x201` frame, then wait approximately 174 ms.
3. Start `0x300`/`0x301`, counter 1, assist Off. Keep command byte 2 at `00` for 99 pairs, then use `64` at approximately 4.95 seconds.
4. Begin the following diagnostic reads at or after 2.148 seconds from the start of the command sequence.
5. Permit assist selection only after the sequencer is running and all eight diagnostic reads have completed.

Shutdown uses two command patterns, with normal checks and companions:

| Phase | Bytes 0..6 | Implemented count |
|---|---|---:|
| First | `00 5A 00 5A 00 00 0A` | 31 pairs |
| Final | Last profile, with byte 2 `00` and byte 3 `A5` | 10 pairs |

Stock captures show the transition to the final pattern after about 1.55 seconds and the last command about 2.00 seconds after the first shutdown pattern. The implementation chooses 41 full pairs, including the final companion, then stops. It does not claim that every stock capture contained exactly those 41 complete pairs.

The application first selects Off and waits five seconds before entering this sequence. Thus its final profile is Off, while a stock capture may retain the previous assist settings in the final pattern. Transport failures and stale motor telemetry stop controller operation; the motor's complete fault-response behavior has not been characterized.

## Startup diagnostics: `0x100` / `0x101`

The [diagnostic client](../protocol/tcu_diagnostics.c) implements the observed UDS/ISO-TP-style read exchange. It sends requests on `0x100` and accepts responses on `0x101`.

```text
Request, DLC 8:  03 22 DID_H DID_L 00 00 00 00
Positive data:  62 DID_H DID_L <value>
Flow control, DLC 3: 30 00 00
```

The DID is big-endian in the request/response header. The initial `03` is the ISO-TP payload length, not a service byte. Responses may be single-frame or segmented. Respect the declared length; trailing CAN padding is not another field.

Reads occur in this order:

| DID | Observed value length | Interpretation |
|---|---:|---|
| `0203` | 20 bytes | Identification data |
| `0202` | 24 bytes | Identification/version data |
| `0204` | 20 bytes | Bike identification |
| `0207` | 2 bytes | Acceleration setting, little-endian; BLE perturbation confirmed |
| `0206` | 2 bytes | Wheel circumference in mm, little-endian; BLE value matched |
| `020D` | 1 byte | Shuttle setting; BLE perturbation confirmed |
| `0213` | 4 bytes | Unknown |
| `0217` | 4 bytes | Unknown |

These lengths describe the observed bike, not a guaranteed schema for other firmware. The client bounds complete responses to 64 bytes, validates sequence numbers and the requested DID, and uses a one-second timeout. It sends read service `22` and flow control only. The research captured writes using service `2E`, but this controller does not expose arbitrary diagnostic writes or motor firmware updates.

## Telemetry

| ID / DLC | Bytes | Interpretation | Evidence |
|---|---|---|---|
| `201` / 6 | 0..1 | uint16 / 100 km/h | Wheel-only test, BLE comparison |
| `201` / 6 | 2..3 | uint16 raw load | Rose under pedal load, fell on release; Nm scale uncalibrated |
| `201` / 6 | 4..5 | Raw status | Bit meanings incomplete; bit 0 correlated with Walk |
| `202` / 8 | 4..7 | uint32 metres | BLE odometer match and wheel-spin increment |
| `203` / 8 | 0..1 | uint16 / 40 rpm | Pedaling/coasting test, BLE comparison |
| `203` / 8 | 2..5 | uint32 power-product candidate | Load/cadence correlation; not integer watts |
| `402` / 8 | 0..1 | Battery percentage candidate | Initial static BLE match; uncalibrated |

Earlier experiments estimated torque as `load_raw × 0.01 Nm` and rider watts from that torque and cadence. Neither scale is calibrated. Backpedaling or lightly moving the cranks produced misleading effort credit in an experimental game. The raw load signal responding to pedal pressure does not establish a reliable forward-work measurement. The reference screen does not calculate or display rider power.

Other IDs remain partly or wholly unresolved. In particular, `401` battery voltage/current scaling comes from outside research and was not independently calibrated here. `403` health/capacity, `404`/`405` battery queries, and several status IDs should not be treated as a complete battery protocol. Raw capture presence does not establish which physical node owns every ID.

## Prior work

This project began with BLE control and used these references alongside local measurements:

- [Jamie Magee, specialized-turbo](https://github.com/JamieMagee/specialized-turbo): Python BLE implementation and protocol documentation.
- [Sepp62, LevoEsp32Ble](https://github.com/Sepp62/LevoEsp32Ble): ESP32 BLE implementation and field descriptions.
- [ReadieFur, OpenTCU](https://github.com/ReadieFur/OpenTCU/tree/research): independent TCU, CAN, and wake-circuit research.

Their findings provided useful starting points. Where local observations differ or remain incomplete, the tables above state the evidence used by this implementation.

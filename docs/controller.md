# Using the protocol in your own controller

`protocol/` has no display, networking or ESP-IDF dependency.

| File | Purpose |
|---|---|
| `levo_protocol.c/.h` | Encode 0x300/0x301; decode observed motion, cadence and distance fields |
| `levo.py` | Offline decoder, command encoder and ISO-TP reassembly |
| `levo-observed.dbc` | Signal definitions for CAN tools |
| `tcu_sequence.c/.h` | Timed startup, running profiles and shutdown pairs |
| `tcu_diagnostics.c/.h` | Eight startup reads and ISO-TP flow control |

The reference ESP-IDF integration is `firmware/main/off_attempt.inc`. `standalone_control.c` handles assist selection, stale motor data and the Off dwell before shutdown. Its `tcu_bench_*` function names are historical; the controller does not need a USB host.

## Integration sequence

1. Supply a monotonic clock and 250 kbit/s classical CAN, using standard frames and exact DLCs.
2. Remove the stock TCU before sending replacement traffic. Two senders of 0x300/0x301 conflict.
3. Wake the bike through the separate [electrical circuit](../hardware/README.md). CAN-only wake has not been demonstrated here.
4. Start Off. Run the command sequencer and startup diagnostic client on schedule. Wait for both to complete startup before allowing assistance.
5. Send returned frames in order and check transmission completion. A queued frame is not proof it reached the bus.
6. Monitor motor traffic and controller errors. The reference controller requests Off and shutdown after 500 ms without valid 0x201 data. Transmission, receive, diagnostic and timing failures stop it. It never retries startup automatically.
7. Gate Walk on a fresh, held input; return to Off on release. Require release before rearming.
8. Select Off, allow the five-second dwell, then run the shutdown sequence before removing power.

Poll the sequencer at least every millisecond. It schedules a command every 50 ms and its companion 1 ms later. More than 10 ms lateness faults rather than sending a burst of overdue frames. These are implementation choices, not measured motor tolerance limits.

The reference firmware conservatively stops on CAN bus-error alerts, including alerts accumulated between sends. That can stop a controller for errors on another node's traffic. Error-injection testing and the motor's complete response to missing commands remain uncharacterized. A transport fault may prevent a final Off frame from reaching the motor.

The [offline Turbo example](../examples/encode_turbo.c) prints a valid pair. It does not implement wake, startup or a continuous control stream.

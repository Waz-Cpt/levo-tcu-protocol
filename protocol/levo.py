"""Measured SBC-D03 wire format. No I/O and no CAN transmission.

Offsets are zero-based. Voltage/current scales are external-source hypotheses;
raw load fields are deliberately not exposed as calibrated torque or watts.
"""
import struct


def command_tail(payload, counter):
    """Observed 0x300 checksum nibble and rolling counter, all captured states.

    Adjacent nibbles overlap with OR, not XOR. This is an empirical model,
    validated against recordings and used by the replacement on one Levo.
    See docs/can-protocol.md for scope and untested payloads.
    """
    if len(payload) != 7 or not 0 <= counter < 16:
        raise ValueError('Need seven payload bytes and counter 0..15')
    check = payload[0] & 15
    for i in range(1, 7):
        check ^= (payload[i - 1] >> 4) | (payload[i] & 15)
    check ^= (payload[6] >> 4) | counter
    return ((check & 15) << 4) | counter


def encode_command(payload, counter):
    payload = bytes(payload)
    return payload + bytes([command_tail(payload, counter)])


def decode(identifier, payload):
    b = bytes(payload)
    if identifier == 0x300 and len(b) == 8:
        return dict(kind='command',counter=b[7]&15,
                    checksum_valid=b[7]==command_tail(b[:7],b[7]&15),
                    state_raw=b[0],walk_raw=b[1],walk_active=b[1]==0xA5,
                    startup_raw=b[2],shutdown_raw=b[3],support_percent=b[4],
                    unknown_5=b[5],peak_percent=b[6])
    if identifier == 0x201 and len(b) == 6:
        return dict(kind='motion',speed_kmh=int.from_bytes(b[:2],'little')/100,
                    load_raw=int.from_bytes(b[2:4],'little'),status_raw=b[4:6].hex())
    if identifier == 0x203 and len(b) == 8:
        return dict(kind='rider',cadence_rpm=int.from_bytes(b[:2],'little')/40,
                    power_product_raw=int.from_bytes(b[2:6],'little'),unknown_6_7=b[6:].hex())
    if identifier == 0x202 and len(b) == 8:
        return dict(kind='distance',odometer_metres=int.from_bytes(b[4:8],'little'),unknown_0_3=b[:4].hex())
    if identifier == 0x401 and len(b) == 8:
        return dict(kind='battery_candidate',voltage_raw=int.from_bytes(b[:2],'little'),
                    current_raw=int.from_bytes(b[4:8],'little',signed=True),unknown_2_3=b[2:4].hex())
    return dict(kind='unknown',hex=b.hex())


class IsoTp:
    """Read-only standard addressing reassembler; no flow-control transmission.

    Uses declared length, sequence validation, and bounded buffers. Feed frames
    in bus order. Captures may start mid-message; incomplete streams are ignored.
    """
    def __init__(self):
        self.pending={}
    def feed(self, identifier, payload, timestamp):
        b=bytes(payload)
        if not b:return None
        kind=b[0]>>4
        if kind==0:
            self.pending.pop(identifier,None)
            n=b[0]&15
            return b[1:1+n] if 0<n<=len(b)-1 else None
        if kind==1:
            # A new First Frame replaces any old stream, even if malformed.
            self.pending.pop(identifier,None)
            if len(b)<2:return None
            n=((b[0]&15)<<8)|b[1]
            if 7<n<=4095:self.pending[identifier]=(n,bytearray(b[2:]),1,timestamp)
            return None
        if kind==2 and identifier in self.pending:
            n,data,expected,last=self.pending[identifier]
            if b[0]&15!=expected or not 0<=timestamp-last<=2:
                del self.pending[identifier];return None
            data.extend(b[1:])
            if len(data)>=n:
                del self.pending[identifier];return bytes(data[:n])
            self.pending[identifier]=(n,data,(expected+1)&15,timestamp)
        return None

# Every observed 0x301 frame matched the immediately preceding 0x300 counter.
# This is a capture-derived table, not a claim about the unknown first-byte algorithm.
COMPANION_FIRST = bytes.fromhex('e2b36b960354f5b88088c643b54e1de8')

def encode_companion(counter):
    if not 0 <= counter < 16:raise ValueError('Counter must be 0..15')
    return bytes([COMPANION_FIRST[counter],counter,0])

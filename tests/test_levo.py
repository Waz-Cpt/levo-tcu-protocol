import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'protocol'))
from levo import encode_command,command_tail,decode,IsoTp,encode_companion

class CodecTests(unittest.TestCase):
 def test_independent_capture_vectors(self):
  for value in ('035a645a230023ea','03a5645a230023ff','005a005a00000a82','035a00a564006421','035a005a32004611'):
   b=bytes.fromhex(value)
   self.assertEqual(encode_command(b[:7],b[7]&15),b)
   self.assertTrue(decode(0x300,b)['checksum_valid'])
 def test_reject_invalid_parameters(self):
  for payload,n in ((b'\0'*6,0),(b'\0'*7,16),(b'\0'*7,-1)):
   with self.assertRaises(ValueError):command_tail(payload,n)
 def test_capture_units_and_malformed_lengths(self):
  self.assertEqual(decode(0x201,bytes.fromhex('100400006200'))['speed_kmh'],10.4)
  self.assertEqual(decode(0x203,bytes.fromhex('bd11000000000000'))['cadence_rpm'],113.525)
  self.assertEqual(decode(0x201,b'\0'*5)['kind'],'unknown')
 def test_companion_capture_vectors(self):
  self.assertEqual(encode_companion(0),bytes.fromhex('e20000'))
  self.assertEqual(encode_companion(15),bytes.fromhex('e80f00'))
 def test_single_frame_ignores_padding(self):
  self.assertEqual(IsoTp().feed(0x101,bytes.fromhex('056202071815e0ac'),1),bytes.fromhex('6202071815'))
 def test_multiframe_trim_and_sequence(self):
  t=IsoTp()
  self.assertIsNone(t.feed(0x101,bytes.fromhex('1009620204414243'),0))
  self.assertEqual(t.feed(0x101,bytes.fromhex('2144454600000000'),.1),bytes.fromhex('620204414243444546'))
  self.assertIsNone(t.feed(0x101,bytes.fromhex('2200000000000000'),.2))
 def test_malformed_first_frame_discards_previous_stream(self):
  for malformed in ('10', '1003deadbeef0000', '1000000000000000'):
   with self.subTest(first_frame=malformed):
    t=IsoTp()
    t.feed(0x101,bytes.fromhex('1009620204414243'),0)
    self.assertIsNone(t.feed(0x101,bytes.fromhex(malformed),.01))
    self.assertFalse(t.pending)
    self.assertIsNone(t.feed(0x101,bytes.fromhex('2144454600000000'),.02))
 def test_new_first_frame_replaces_only_its_identifier(self):
  t=IsoTp()
  first=bytes.fromhex('1009620204414243')
  t.feed(0x101,first,0)
  t.feed(0x201,first,0)
  t.feed(0x101,bytes.fromhex('1003deadbeef0000'),.01)
  self.assertIn(0x201,t.pending)
  t.feed(0x101,bytes.fromhex('100962020458595a'),.02)
  self.assertEqual(t.feed(0x101,bytes.fromhex('2131323300000000'),.03),bytes.fromhex('62020458595a313233'))
 def test_wrong_sequence_and_timeout(self):
  for following,stamp in (('2244454600000000',.1),('2144454600000000',3)):
   t=IsoTp();t.feed(0x101,bytes.fromhex('1009620204414243'),0)
   self.assertIsNone(t.feed(0x101,bytes.fromhex(following),stamp))
   self.assertFalse(t.pending)
if __name__=='__main__':unittest.main()

#!/usr/bin/env python3
"""Render the reference module-level wiring schematic. No third-party packages."""
from html import escape
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'hardware/diagrams/reference-schematic.svg'
p=['<svg xmlns="http://www.w3.org/2000/svg" width="1500" height="1160" viewBox="0 0 1500 1160" role="img" aria-labelledby="title desc">',
'<title id="title">Levo TCU reference wiring schematic</title>',
'<desc id="desc">Five-pin male bike connector, six-pin female remote connector, 12-to-5 volt buck, Waveshare ESP32-C6 display and a 3.3 volt CAN transceiver. Pin numbering follows the separate mating-face drawings. Named nets connect electrically; no wire colors are specified.</desc>',
'<rect width="1500" height="1160" fill="white"/>']
def text(x,y,s,size=20,bold=False,anchor='start',color='#17232b'):
 p.append(f'<text font-family="sans-serif" x="{x}" y="{y}" font-size="{size}" font-weight="{700 if bold else 400}" text-anchor="{anchor}" fill="{color}">{escape(s)}</text>')
def line(*pts):
 p.append('<polyline points="'+' '.join(f'{x},{y}' for x,y in pts)+'" fill="none" stroke="#17232b" stroke-width="2.5"/>')
def box(x,y,w,h,title,subtitle):
 p.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="#f7f9fb" stroke="#17232b" stroke-width="2"/>')
 text(x+16,y+30,title,22,True);text(x+16,y+55,subtitle,17,color='#52616b')
def dot(x,y):p.append(f'<circle cx="{x}" cy="{y}" r="4.5" fill="#17232b"/>')
def ground(x,y):
 line((x,y),(x,y+12));line((x-12,y+12),(x+12,y+12));line((x-8,y+18),(x+8,y+18));line((x-3,y+24),(x+3,y+24))
def resistor(x,y,horizontal=True,length=60):
 if horizontal:p.append(f'<rect x="{x}" y="{y-8}" width="{length}" height="16" fill="white" stroke="#17232b" stroke-width="2"/>')
 else:p.append(f'<rect x="{x-8}" y="{y}" width="16" height="{length}" fill="white" stroke="#17232b" stroke-width="2"/>')
def net(x,y,label,anchor='start'):text(x,y-9,label,18,True,anchor)
text(50,48,'Levo TCU · reference wiring',34,True)
text(50,82,'Module-level schematic · no cable-color assumptions · use the keyed mating-face pinout drawings',20)
text(50,110,'Matching net labels are connected. Dots mark junctions. U2A and U2B are the same ESP32 board.',18,color='#52616b')
box(50,140,300,350,'J1 · BIKE','5-pin MALE on replacement')
for n,label,y in [(1,'Switched ~12 V',215),(2,'Ground',265),(3,'CAN H',315),(4,'CAN L',365),(5,'Wake / pack-level',415)]:
 text(68,y+7,f'{n}   {label}',20);line((320,y),(350,y))
box(520,140,260,180,'U1 · BUCK','Common-ground 12 V → 5 V')
text(535,221,'IN+',18);text(535,271,'IN−',18)
text(765,221,'OUT+',18,anchor='end');text(765,271,'OUT−',18,anchor='end')
line((350,215),(520,215));net(390,215,'+12_SW')
line((350,265),(400,265));ground(400,265)
line((520,265),(490,265));ground(490,265)
line((780,265),(810,265));ground(810,265)
box(1050,140,400,550,'U2A · ESP32 + DISPLAY','Waveshare ESP32-C6-Touch-LCD-1.47')
text(1070,221,'5V / VBUS',20);text(1070,271,'GND',20)
text(1070,371,'3V3 output',20);text(1070,471,'GPIO17 → CAN transmit',20);text(1070,531,'GPIO7 ← CAN receive',20)
text(1070,600,'172 × 320 screen and touch',18)
text(1070,630,'are already on this board.',18)
text(1070,660,'Leave the microSD slot empty.',18,True)
line((780,215),(880,215));p.append('<rect x="880" y="205" width="52" height="20" fill="white" stroke="#17232b" stroke-width="2"/>');line((888,215),(924,215));line((932,215),(1050,215))
text(915,171,'JP1 · removable 5 V link',18,True,'middle');text(915,193,'Open before USB power',17,False,'middle')
line((1050,265),(1015,265));ground(1015,265)
line((1050,365),(1000,365));net(1030,355,'+3V3','end')
line((350,315),(450,315),(450,465),(520,465))
line((350,365),(420,365),(420,525),(520,525))
line((350,415),(390,415));net(360,438,'WAKE')
box(520,400,280,240,'U3 · CAN MODULE','');text(535,447,'3.3 V supply AND logic',17)
text(535,471,'CAN H',19);text(535,531,'CAN L',19)
text(785,471,'CTX / TXD',19,anchor='end');text(785,531,'CRX / RXD',19,anchor='end')
text(680,390,'VCC',17);line((660,400),(660,370));net(660,370,'+3V3','middle')
text(660,625,'GND',17,False,'middle');line((660,640),(660,655));ground(660,655)
line((800,465),(1050,465));line((800,525),(1050,525))
net(1000,465,'CAN_TX','end');net(1000,525,'CAN_RX','end')
# One pull-up from the input of the transceiver to the board's 3.3 V supply.
line((900,465),(900,425));resistor(900,375,False,50);line((900,375),(900,350));net(900,350,'+3V3','middle');dot(900,465)
text(920,390,'R3',18,True);text(920,415,'10 kΩ',18)
# A single end termination. Do not add if the module already contains it.
line((480,465),(480,480));resistor(480,480,False,30);line((480,510),(480,525));dot(480,465);dot(480,525)
text(340,562,'R1 · 120 Ω*',19,True)
# Electrical wake path is separate from all GPIOs.
net(60,625,'WAKE');line((60,625),(130,625));resistor(130,625,True,60);line((190,625),(235,625));dot(235,625);dot(295,625);line((235,625),(285,601));line((295,625),(325,625));ground(325,625)
text(160,590,'R2 · 10 kΩ / 0.5 W',18,True,'middle');text(235,675,'SW1 · momentary, normally open',17,False,'middle')
text(50,714,'Wake circuit only. Never connect WAKE or +12_SW to an ESP GPIO.',20,True)
text(50,747,'* R1: fit one 120 Ω terminator here only if U3 has none. All ground symbols share bike ground.',18)
line((50,770),(1450,770))
box(50,790,330,270,'J2 · HANDLEBAR REMOTE','6-pin FEMALE on replacement')
box(1050,790,400,270,'U2B · REMOTE INPUTS','Same board as U2A above')
for n,label,gpio,y in [(1,'Walk',16,865),(3,'Specialized',8,935),(4,'Plus',3,975),(5,'Minus',4,1015)]:
 text(68,y+7,f'{n}   {label}',20);line((350,y),(1050,y));text(1070,y+7,f'GPIO{gpio}',20)
text(68,900,'2   Unused',20);line((350,895),(420,895));line((414,889),(426,901));line((414,901),(426,889));text(450,901,'Insulate this conductor; no connection',18)
text(68,1052,'6   Button common',20);line((350,1045),(520,1045));resistor(520,1045,True,60);line((580,1045),(630,1045));ground(630,1045);text(730,1052,'R4 · 220 Ω',18,True)
text(1070,1045,'Internal pull-ups enabled in firmware',17)
text(50,1110,'R1, R3, R4: 0.25 W or greater. R2: 0.5 W. 1% resistors make sensible new-build defaults.',18)
text(50,1138,'Reference wiring for the tested harness. Verify connector family, keying and continuity on your own pigtails.',18,color='#52616b')
p.append('</svg>')
OUT.write_text('\n'.join(p)+'\n')
print(OUT)

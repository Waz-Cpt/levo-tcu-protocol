#!/usr/bin/env python3
"""Generate connector-face SVGs and Markdown from hardware/connectors.json.

Uses the Python standard library. Coordinates follow the builder's keyed-face
drawings; the opposite mating face is reflected, not rotated.
"""
import json
from html import escape
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DATA = json.loads((ROOT / "hardware/connectors.json").read_text())
OUT = ROOT / "hardware/diagrams"
OUT.mkdir(exist_ok=True)

# The female face has 1 at upper left and 2 at upper right. The male
# mating face mirrors it horizontally. Pin 6 is the center contact.
FEMALE = {1: (-65, -55), 2: (65, -55), 3: (65, 40),
          4: (0, 85), 5: (-65, 40), 6: (0, 5)}


def svg_for(connector):
    parts = [
        '<svg xmlns="http://www.w3.org/2000/svg" width="1200" height="900" '
        'viewBox="0 0 1200 900" role="img" aria-labelledby="title desc">',
        f'<title id="title">{escape(connector["title"])}: both mating faces</title>',
        '<desc id="desc">Looking into the keyed mating faces, with the key at '
        '12 o’clock. Male pins and female sockets are mirror images. '
        'The table identifies each numbered contact.</desc>',
        '<rect width="1200" height="900" fill="#ffffff"/>',
        '<g font-family="sans-serif" fill="#17232b">',
    ]

    def text(x, y, value, size=22, weight="400", anchor="start", color="#17232b"):
        parts.append(f'<text font-family="sans-serif" x="{x}" y="{y}" font-size="{size}" font-weight="{weight}" '
                     f'text-anchor="{anchor}" fill="{color}">{escape(str(value))}</text>')

    text(40, 54, f'{connector["title"]} · {connector["count"]}-pin Julet', 34, "700")
    text(40, 89, DATA["view"], 21, color="#52616b")

    for sex, cx, left in (("male", 300, 40), ("female", 900, 640)):
        parts.append(f'<rect x="{left}" y="116" width="520" height="348" rx="14" '
                     'fill="#f5f8fa" stroke="#d8e1e7"/>')
        text(cx, 154, connector[sex + "_role"], 25, "700", "middle")
        text(cx, 184, "MALE · PINS" if sex == "male" else "FEMALE · SOCKETS",
             17, "700", "middle", "#52616b")
        cy = 323
        parts.append(f'<circle cx="{cx}" cy="{cy}" r="112" fill="#ffffff" '
                     'stroke="#17232b" stroke-width="3"/>')
        if sex == "female":
            parts.append(f'<circle cx="{cx}" cy="{cy}" r="124" fill="none" '
                         'stroke="#8b9ba7" stroke-width="2"/>')
        # A schematic notch identifies orientation, not the keyed profile's dimensions.
        parts.append(f'<path d="M {cx-21} {cy-112} L {cx-21} {cy-95} '
                     f'Q {cx} {cy-68} {cx+21} {cy-95} L {cx+21} {cy-112}" '
                     'fill="#f5f8fa" stroke="#17232b" stroke-width="3"/>')
        for pin in connector["pins"]:
            n = pin["number"]
            dx, dy = FEMALE[n]
            x, y = cx + (dx if sex == "female" else -dx), cy + dy
            fill, ink = ("#ffffff", "#17232b") if sex == "female" else ("#17232b", "#ffffff")
            parts.append(f'<circle cx="{x}" cy="{y}" r="20" fill="{fill}" '
                         'stroke="#17232b" stroke-width="2.5"/>')
            text(x, y + 8, n, 23, "700", "middle", ink)

    text(40, 505, "Pin", 18, "700", color="#52616b")
    text(122, 505, "Signal", 18, "700", color="#52616b")
    text(430, 505, "Connection in reference build", 18, "700", color="#52616b")
    parts.append('<path d="M40 519 H1160" stroke="#17232b" stroke-width="2"/>')
    for row, pin in enumerate(connector["pins"]):
        y = 552 + row * 42
        text(52, y, pin["number"], 22, "700")
        text(122, y, pin["signal"], 22)
        text(430, y, pin["connection"], 20)
        parts.append(f'<path d="M40 {y+12} H1160" stroke="#e0e7ec"/>')

    text(40, 834, "Face view, not wire-entry view. Match the key before identifying contacts.", 19, "700")
    text(40, 865, "Project reference numbering. Cable colors vary. Diagram is not to scale.", 18, color="#52616b")
    parts.append('</g></svg>')
    return '\n'.join(parts) + '\n'


def battery_svg(connector):
    parts = [
        '<svg xmlns="http://www.w3.org/2000/svg" width="1200" height="900" '
        'viewBox="0 0 1200 900" role="img" aria-labelledby="title desc">',
        f'<title id="title">{escape(connector["title"])}</title>',
        '<desc id="desc">Bike-side plug viewed into its mating face, cable downward. '
        'The lower-left contact is CAN L; the lower-right is CAN H. '
        'The remaining contacts have no assigned function in this drawing.</desc>',
        '<rect width="1200" height="900" fill="#ffffff"/>',
    ]

    def text(x, y, value, size=22, weight="400", anchor="start", color="#17232b"):
        parts.append(f'<text font-family="sans-serif" x="{x}" y="{y}" '
                     f'font-size="{size}" font-weight="{weight}" text-anchor="{anchor}" '
                     f'fill="{color}">{escape(value)}</text>')

    text(40, 54, connector['title'], 34, '700')
    text(40, 89, connector['view'], 21, color='#52616b')
    parts.append('<rect x="40" y="116" width="1120" height="562" rx="14" '
                 'fill="#f5f8fa" stroke="#d8e1e7"/>')
    text(600, 157, connector['orientation'], 23, '700', 'middle')
    parts += [
        '<path d="M570 519 V608 H630 V519" fill="#ffffff" stroke="#8b9ba7" stroke-width="3"/>',
        '<circle cx="600" cy="360" r="164" fill="#ffffff" stroke="#8b9ba7" stroke-width="3"/>',
        '<circle cx="600" cy="360" r="144" fill="#ffffff" stroke="#17232b" stroke-width="3"/>',
        '<circle cx="550" cy="360" r="27" fill="#f5f8fa" stroke="#8b9ba7" stroke-width="3"/>',
        '<circle cx="650" cy="360" r="27" fill="#f5f8fa" stroke="#8b9ba7" stroke-width="3"/>',
        '<circle cx="550" cy="280" r="10" fill="#8b9ba7"/>',
        '<path d="M640 270 L660 290 M660 270 L640 290" stroke="#8b9ba7" stroke-width="3"/>',
        '<circle cx="550" cy="440" r="12" fill="#17232b"/>',
        '<circle cx="650" cy="440" r="12" fill="#17232b"/>',
        '<path d="M550 440 L480 480 H350 M650 440 L720 480 H850" '
        'fill="none" stroke="#17232b" stroke-width="2.5"/>',
        '<path d="M600 546 V585 M592 575 L600 585 L608 575" '
        'fill="none" stroke="#52616b" stroke-width="2"/>',
    ]
    text(340, 472, connector['lower_left'], 29, '700', 'end')
    text(340, 509, 'Lower left', 20, anchor='end', color='#52616b')
    text(860, 472, connector['lower_right'], 29, '700')
    text(860, 509, 'Lower right', 20, color='#52616b')
    text(600, 644, 'Cable to bike', 21, anchor='middle', color='#52616b')
    text(40, 728, 'The battery receptacle is not shown. Its mating face is mirrored horizontally.', 23, '700')
    text(40, 768, 'Other contacts are not assigned. X is retained from the source sketch;', 21, color='#52616b')
    text(40, 797, 'its meaning is unspecified. No contact numbers or power polarity are inferred.', 21, color='#52616b')
    text(40, 865, 'Reference geometry only. Diagram is not to scale.', 18, color='#52616b')
    parts.append('</svg>')
    return '\n'.join(parts) + '\n'


lines = [
    '# Connector pinouts',
    '',
    '**Julet views: looking into the mating face, with the key at 12 o’clock.** '
    'The separate battery plug is shown cable-down. These are not wire-entry views.',
    '',
    'Pin numbers are project references transcribed from the builder’s keyed-face '
    'drawings. They are not a claimed manufacturer numbering standard. Signal assignments '
    'combine those drawings with the tested reference harness. Verify your own connector '
    'and cable continuity; pigtail colors are not standardized.',
    '',
    'The Signal column describes the harness. ESP GPIO numbers are choices made for this reference build, not requirements of the bike protocol. Match each contact to its wire using continuity; ignore insulation colors.',
    '',
]
for connector in DATA['connectors']:
    name = connector['id'] + '.svg'
    (OUT / name).write_text(svg_for(connector))
    lines += [f'## {connector["title"]}', '',
              f'![{connector["title"]}, male and female mating faces](diagrams/{name})', '',
              '| Pin | Signal | Connection in reference build |', '|---|---|---|']
    for pin in connector['pins']:
        lines.append(f'| {pin["number"]} | {pin["signal"]} | {pin["connection"]} |')
    lines += ['']
(OUT / 'battery-bike-side.svg').write_text(battery_svg(DATA['battery_connector']))
lines += [
    'The wake line is not an ESP input. See the [hardware guide](README.md) for '
    'power, termination, the wake switch and remote pull-ups.',
    '',
    '## Battery connector',
    '',
    '![Bike-side battery plug mating face, cable downward](diagrams/battery-bike-side.svg)',
    '',
    'This is the **plug on the bike that inserts into the battery**, viewed from its '
    'mating face with the cable downward. CAN L is the lower-left contact and CAN H '
    'is the lower-right. The battery itself is not drawn; its mating face would be '
    'mirrored horizontally.',
    '',
    DATA['battery_connector']['unmapped'] + ' No contact numbers or power polarity are inferred.',
    '',
    '## Regenerate',
    '',
    'Edit `hardware/connectors.json`, then run from the repository root:',
    '',
    '```sh',
    'python3 tools/generate_connector_diagrams.py',
    '```',
    '',
    'This writes the SVGs and this page using only the Python standard library. '
    'The original reference photos remain outside the public repository.',
    '',
]
(ROOT / 'hardware/pinouts.md').write_text('\n'.join(lines))

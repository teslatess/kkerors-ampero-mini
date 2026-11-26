#!/usr/bin/env python3
"""Check served static assets against files. Usage: check_site.py http://localhost:8765"""
import sys
from pathlib import Path
from urllib.request import urlopen
root=Path(__file__).resolve().parents[1]
for name in ['index.html','style.css','app.js','patcher.js','patches/V2.2.json','images/mode-2.svg','images/mode-3.svg','images/display-modes.svg']:
    with urlopen(sys.argv[1].rstrip('/')+'/'+name,timeout=10) as response:
        assert response.status==200
        assert response.read()==(root/'docs'/name).read_bytes()
print('PASS: all site assets served successfully and matched files.')
for p in root.rglob('*'):
    if p.is_file():
        assert p.suffix.lower() not in {'.bin','.zip','.bundle','.elf','.o','.clo','.tone','.namb'},p
print('PASS: no firmware images or private binary archives in publication tree.')

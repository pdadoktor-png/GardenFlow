#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
IGNORED = {'.git', '.pio', '.vscode', '__MACOSX'}
issues = []
seen = {}
for path in ROOT.rglob('*'):
    if not path.is_file() or any(part in IGNORED for part in path.parts):
        continue
    rel = path.relative_to(ROOT)
    key = str(rel).lower()
    if key in seen and seen[key] != rel:
        issues.append(f'CASE-DUPLIKAT: {seen[key]} <-> {rel}')
    else:
        seen[key] = rel

entry_points=[]
for path in (ROOT/'src').rglob('*.cpp'):
    text=path.read_text(encoding='utf-8',errors='ignore')
    if 'void setup()' in text and 'void loop()' in text:
        entry_points.append(path.relative_to(ROOT))
if len(entry_points)>1:
    issues.append('MEHRERE ENTRYPOINTS: '+', '.join(map(str,entry_points)))

for path in ROOT.iterdir():
    if path.is_dir() and path.name.startswith('GardenFlow-v'):
        issues.append(f'ALTE PROJEKTKOPIE IM REPO: {path.name}')
    if path.is_file() and path.suffix.lower()=='.zip':
        issues.append(f'ZIP IM REPO: {path.name}')

print('GardenFlow Projekt-Audit')
print('========================')
if not issues:
    print('OK: keine strukturellen Auffaelligkeiten gefunden.')
else:
    for issue in issues:
        print('WARN:',issue)
    print(f'\n{len(issues)} Hinweis(e). Der Audit veraendert keine Dateien.')
sys.exit(0)

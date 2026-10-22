from pathlib import Path
import zipfile,hashlib,json
root=Path('port/out/stage/infamous-linux-x86_64')
readme="""Karma Switcher - Linux x86_64 TEST BUILD
Built on Ubuntu 24.04. Linux gameplay has NOT been tested.
Supports CUSA00223 update 1.07 with exact game-code checks.
Includes True Hero / Infamous Level 5 switching (Insert -> Karma), plus
camera 1.5x / diagonal slowdown, cursor and performance menu fixes.
Enable the existing Camera speed 1.5x mod in the launcher for camera changes.

FULL ZIP: Extract to a NEW folder. Set the launcher executable if necessary:
chmod +x 'inFAMOUS Second Son' 'inFAMOUS Second Son (classic)' out/tlg-probe out/tlg-probe-v2
Point the launcher at your own game files and use a Renderer v2 preset.
No game files or saves are included.

UPDATE ZIP: For matching cde38af-based Linux launchers only. Close the game
and launcher. Back up out/tlg-probe-v2 AND out/gpu_v2/libissgpu.so. Extract
the ZIP into the existing launcher folder, replacing both files.
Run chmod +x out/tlg-probe-v2 if your ZIP extractor loses executable bits.

Back up your saves before testing. The switcher also requires a verified
save backup before each request, stored in <data folder>/karma-backups.
Story power families must already be earned. Switching grants eligible
upgrades for the selected karma side; normal saves may record these changes.
Check both modes, a matching ability in each, camera horizontal and diagonal
movement, and the Performance tab. Report distro, GPU, and run log on failure.
"""
(root/'KARMA-SWITCHER-README.txt').write_text(readme)
(root/'KARMA_SWITCHER.md').write_bytes(Path('docs/KARMA_SWITCHER.md').read_bytes())
dist=Path('port/dist')
with zipfile.ZipFile(dist/'karma-switcher-linux-full-test.zip','w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for p in sorted(root.rglob('*')):
        if p.is_file():z.write(p,Path(root.name)/p.relative_to(root))
with zipfile.ZipFile(dist/'karma-switcher-linux-update-test.zip','w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for name in ['out/tlg-probe-v2','out/gpu_v2/libissgpu.so','KARMA-SWITCHER-README.txt','KARMA_SWITCHER.md']:
        z.write(root/name,name)
report={p.name:{'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in dist.glob('karma-switcher-linux-*.zip')}
(dist/'karma-switcher-linux-sha256.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))

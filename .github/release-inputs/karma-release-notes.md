Karma Switcher adds **Insert > Karma > True Hero / Infamous** for maximum Good or Evil karma (Level 5), with eligible upgrades for powers already earned. Switch during gameplay and close the overlay to apply.

Includes the existing camera 1.5x and diagonal slowdown fixes, cursor fix, and Performance menu fix. Enable Camera speed 1.5x in the launcher for camera changes. Supports **CUSA00223 update 1.07**, with exact code checks and verified save backups before switching.

### Downloads
- **karma-switcher-windows-update.zip**: exact Windows runtime package tested in gameplay. Close the game/launcher, back up `out/tlg-probe-v2.exe`, and extract into a matching cde38af-based Windows launcher folder, replacing that file.
- **karma-switcher-linux-full-test.zip**: complete Linux x86_64 launcher/runtime package. Extract into a new folder, then select your own game files and a Renderer v2 preset.
- **karma-switcher-linux-update-test.zip**: update for a matching cde38af-based Linux launcher. Back up and replace both `out/tlg-probe-v2` and `out/gpu_v2/libissgpu.so`. If needed, run `chmod +x out/tlg-probe-v2` after extracting.
- **SHA256SUMS.txt**: download integrity hashes.

### Test status
Windows: both karma modes and branch ability tests passed in gameplay alongside the camera fixes.
Linux: compilation, automated runtime/backup tests, and launcher startup checked; **Linux gameplay testing is still needed**. Some GPU tests may skip when the runner has no Vulkan driver.

Back up saves before testing. Normal autosaves may record karma and granted upgrades. The switcher does not grant story-locked power families or directly edit mission/collectible fields. Video summons, save/reload, and extended story-progress checks are not fully verified.

No game files, console firmware, or personal saves are included. This is a standalone test release.

# Karma Switcher â€” experimental Windows test

Supports CUSA00223, game update 1.07. Built from clean source at cde38af on branch
`fix/karma-switcher`; retains the previously delivered cursor, camera and FPS fixes.
The simultaneous-all-powers experiment is not included.

Open the Insert-key overlay, select **Karma**, then **True Hero** or **Infamous**.
The button first creates and verifies a backup. Resume gameplay for the queued
change to apply. No restart is required. The chosen mode keeps karma at its maximum
for the remainder of this process. Restarting returns to normal karma behavior;
normal game saves may already have recorded the changed karma and granted upgrades.

- True Hero uses the game's normal score setter with +4300: Good side, rank 5.
- Infamous uses the same function with -4300: Evil side, rank 5.
- Only named, non-prototype upgrades in earned power families are considered.
- Native metadata determines which karma branch and rank are eligible. Base power
  families and quest controls are excluded; script-forced-off flags are respected.
- Normal game ownership setters grant eligible upgrades and send change events.
  Opposing karma restrictions are left intact, so mutually exclusive effects follow
  the selected branch rather than both being forced on together.
- Story powers are not granted ahead of their story unlocks. Story completion,
  missions and collectible fields are not directly edited.

## Compatibility and backups

The loader checks title/update and exact signatures of the camera entry, karma
setter, ownership setter and normal karma eligibility function before installing
anything. A mismatch leaves the switcher disabled. The hook derives addresses from
the loaded image; it does not use a Cheat Engine process address or patch save offsets.

Before each button request, all files under `savedata/1/CUSA00223` in the active data
folder are copied into a new `karma-backups/<timestamp>-<mode>` directory. Existing
backup directories are never overwritten. Every file is compared byte-for-byte and
its source size/time is rechecked. Links, empty saves, copying errors or detected
concurrent changes cancel the switch. A complete backup has
`KARMA_BACKUP_VERIFIED.txt`; incomplete attempts do not.

To restore, close the game and launcher. Preserve the current saves separately,
then restore the contents of a verified backup (excluding the marker) into the
same title save folder. Keep `*.sce_sys` metadata beside its corresponding slot.

## Verified research

The native getter at image + 0xeb270 reads the float at attributes + 0xc0.
Attributes are at image + 0xefacf8. The observed address 0x800efadb8 is therefore
image base 0x800000000 plus that verified field offset. A live True Hero save read
4300, side 1, rank 5; side and rank are adjacent dwords at +0xc4/+0xc8.
Native rank thresholds end at 3900 and 4300. The normal score setter at +0x1e4040
recalculates side/rank and emits the normal change events. Save offsets 0x04,
0x0c and 0x14 have not been assumed or patched.

The overlay submits an atomic request after backup. A guarded callback runs on the
game's camera-update thread, invokes native functions, and publishes an atomic
status snapshot. Overlay/background threads never write guest state. The entry
bridge saves flags, every GPR, x87/SSE state and all 16 full YMM registers.

## Validation and remaining live checks

Windows executable builds successfully from this workspace. Native tests cover
both modes, repeated switches, eligible/opposing upgrades, unavailable families,
script restrictions, untouched quest/collectible canaries, error handling,
signature mismatches/truncation, full YMM/flags preservation, verified backups and
backup failure protection. Tests use instrumented native-function fixtures; they
do not substitute for gameplay validation.

Live validation: the user confirmed that switching both ways through the overlay
and testing the selected branch works. Read-only inspection observed both +4300 /
Good / rank 5 and -4300 / Evil / rank 5, with no runtime switch error. Backups were
created for the switches, with verified-complete markers. All 15 original save and
metadata files still match the pre-test SHA-256 manifest.

Remaining checks: Video summons specifically, save/reload, and story/collectible
progress across extended play. Validation so far used an isolated copy of user
data; it does not imply that a public release has been published.

# Ampero Mini backup guide

Before installing experimental firmware, save your sounds and keep the original Hotone firmware.
A firmware file is not a backup of your personal presets.

## Presets

1. Save any unsaved edits on the pedal.
2. Connect the Mini by USB in normal operation, not firmware-update mode.
3. Open a compatible Ampero Editor and wait for the user preset list to load.
4. Use **Export → Export All Patches** to export the user bank. Menu wording may vary by editor version.
5. Save the exported `.prst` file outside this project and keep a second copy on another drive or in cloud storage.
6. Check that the exported file exists and is not empty. This is a basic check, not a complete restore test.
7. Export particularly important presets individually as well, using **Export One Patch** if available.

Do not use Import, Reset or Factory Reset just to test a backup: these may overwrite data.

## Save separately

- Original IR files: a preset export does not include the impulse-response files themselves.
- Original Sound Clone files: do not assume they can be extracted back from the pedal.
- A list or photos of the IR and Sound Clone slot assignments.
- Photos of important Global, I/O, Display, footswitch and expression settings.
- The original official Hotone V2.2 firmware and its installation instructions.

If you use Dubshift, do not assume a normal preset export includes its separate scenes or all extension settings.
Record those settings separately before changing firmware.

## Restoring

Install compatible firmware, connect in normal mode and import your saved preset file through Ampero Editor.
Importing a whole bank may overwrite existing user presets. Restore IR and Sound Clone files to their original slots
and reapply global settings as needed. Recovery from kkerors mod v1.0 has not been confirmed on hardware.
Follow the official Hotone recovery/update procedure for your device.

## References

- [Hotone Ampero Mini downloads and support](https://www.hotone.com/products/multi-effects/Ampero%20Mini)
- [Pitik documentation](https://github.com/RakiRarufu/pitik-ampero-mini)
- [Dubshift documentation](https://dubshift.dev/patch)

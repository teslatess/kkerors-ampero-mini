# kkerors mod

Independent experimental modification for Hotone Ampero Mini, based on stock V2.2.
Baseline: v1.0 — Mode 3 with assignable A/B presets and BUTTON COLOR.

## Project status

Public repository preparation: browser patcher implemented; publication pending.
Complete the release review before enabling GitHub Pages or advertising a build.
Software validation of the local baseline passed; hardware verification is not confirmed.

## Distribution design

This repository contains source code, not original or modified firmware images.
The planned browser patcher will accept an exact official firmware file supplied by the user,
apply changes locally, and verify the generated file before download.
Stock UI templates must be copied from the user's input at patch time, not bundled with the patch.

## Source and local development

`source/menu/` contains the existing baseline builder, C/ARM code and software checks.
The builder requires an independently obtained, exact stock V2.2 file, Python 3.9+ and clang.
The instruction checks also require a compatible objdump.
Run from the repository root, with a new output path outside this repository:

```sh
python3 source/menu/build.py '/path/to/official-V2.2.bin' '/path/to/new-output.bin'
python3 source/menu/check.py '/path/to/official-V2.2.bin'
clang -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined source/menu/mode3_test.c -o /tmp/kkerors-mod-test
/tmp/kkerors-mod-test
```

Before installation, read [the backup guide](docs/BACKUP_GUIDE.md).

## Public release preparation

1. Clean source-only repository layout: prepared.
2. Technical source inventory: documented in [SOURCE_REVIEW](docs/SOURCE_REVIEW.md); legal clearance is not established.
3. Generate patch operations without bundled stock templates: implemented and verified against v1.0.
4. Local browser patcher: implemented; JavaScript engine matches saved baseline byte-for-byte.
5. User-facing instructions: included. Browser visual/file-picker QA and publication: pending.

## Attribution and independence

See [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
This project is not affiliated with, sponsored by, or endorsed by Hotone.
Hotone and Ampero names identify compatibility. No rights to proprietary firmware are granted.

## Patch generator

See [PATCH_FORMAT](docs/PATCH_FORMAT.md) for generation and local integration checks.
The published recipe is `docs/patches/v1.0.json`; it is not a complete firmware image.

## Local website preview

```sh
python3 -m http.server 8765 --bind 127.0.0.1 --directory docs
```

Open http://localhost:8765. Use HTTPS for deployment. Opening index.html directly
with file:// is not supported (module loading, manifest fetch and Web Crypto).
No runtime dependencies, remote fonts, analytics or firmware upload endpoints are used.
The browser fetches the public patch manifest and processes the selected file locally.
The generated file is verified against the release hash pinned in patcher.js.

## Validation

```sh
npm test
node tests/check_browser_patch.mjs '/path/to/official-V2.2.bin' '/path/to/local-v1.0.bin'
```

Public CI checks metadata and rejects invalid inputs without proprietary firmware.
The local integration check additionally compares every output byte with the saved release,
checks input immutability and rejects 18 malformed manifests and repeated patching.
The Python integration suite remains available for independent generation verification.
No dependency installation is needed for these Node.js 22+ checks.
Browser visual, native file-picker and download QA must be performed before release.

## Planned GitHub Pages deployment

When the release review is complete, select the main branch and /docs folder in
repository Settings → Pages. Use relative paths so project-subpath hosting works.
Publish source and the patch recipe only, never generated firmware in release assets.
The included workflow checks tracked filenames but does not audit past Git history or
external release assets: inspect those separately before making the repository public.

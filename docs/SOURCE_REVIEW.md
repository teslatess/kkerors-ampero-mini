# Technical source/distribution review — 2026-11-27

Scope: current source/menu files and generated v1.0 patch. This is a technical inventory,
not a legal opinion or proof of permission from a manufacturer.

## Findings

- The public tree contains no complete original or modified firmware, firmware archive,
  Git bundle, vendor font, logo image or audio model asset.
- build.py reads six UI/template regions from the user's exact original firmware.
  The manifest stores only their offsets, lengths and our record edits. All 1120 bytes
  are zero in the published compiled payload before runtime copying.
- mode3.c implements extension UI/input/storage logic and calls firmware services at fixed
  addresses. It contains numeric GUI/font addresses, palette values and API layouts.
- mode3_bridge.s contains integration trampolines and extension widget definitions.
  Some trampolines reproduce displaced native instruction semantics.
- button_color.s contains extension/menu code, constants and fixed service addresses.
- build.py contains short expected hook instruction/data values (2–6 bytes per site).
  The public manifest uses hashes for these preconditions; source retains the values
  needed by the existing builder. They are not a full firmware or UI asset dump.
- check.py and mode3_test.c contain instruction/API fixtures, font-width metrics and mocked
  service behavior. Their fixture provenance is part of the release review, not excluded
  merely because the files are tests.
- The existing MIT notice credits RakiRarufu and is preserved verbatim. The recorded
  upstream commit identifies project ancestry; it does not establish authorship of every
  later local change. No additional author identities have been invented.

## Verification

The generated recipe reproduces the saved v1.0 binary byte-for-byte:
`de0fd0b6e61b626939da3ddacd42922cf1e5fdc70131d97fb5e101b09b62e488`.
Regeneration is deterministic with the tested compiler. The integration test checks
six empty template regions, absence of those complete stock regions in the raw payload,
input preservation, and 17 invalid input/operation rejection cases.
These checks do not prove that no short byte pattern matches firmware; machine code
naturally shares instruction encodings. They also do not establish copyright clearance.

## Before publication

Review retained instruction fragments, numeric tables/font metrics and authorship/license
coverage in context. A browser patcher reduces bundled proprietary content but does not
itself grant permission to modify firmware or override applicable license terms.
No blanket claim of being free from third-party rights is made.

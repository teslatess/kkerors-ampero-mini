## 2026-11-27

- Use stock V2.2 as the only public firmware version; output filename is V2.2_kkerors-mod.bin.
- Replace the abstract A/B cards with illustrated stock Mode 2 and custom Mode 3 screens.
- Simplify the website and project description; firmware bytes are unchanged.

## Custom V2.2 — 2026-11-18

- Changed ASSIGN to SAVE; kept the native Drum uppercase < BACK button and PRESET A/B header.
- Replaced swatches and COLOR A/B/value captions with the exact Drum footer label and selector records, changing only widget IDs. COLOR occupies the first of the three native footer cells; two remain empty.
- COLOR uses the standard seven-name selector with bounded left/right taps. BACK cancels both pending preset and color; SAVE commits them together. List gestures retain native handling even when dragged across the footer.
- Preserved seven A/B and BUTTON COLOR choices, legacy storage, short A/B switching, inert individual footswitch holds and native combined-switch tuner dispatch.
- Host checks cover native selector updates, boundaries, cancelled gestures, zero-coordinate releases and list gesture ownership, together with existing input/storage regressions. ARM checks verify exact Drum template reuse and inverse container reconstruction.
- Software checks passed. This file has not been installed or visually verified on hardware.

## Revision 5 — 2026-11-18

- Implemented the approved palette chooser: PRESET A/B header, native Drum < BACK (record 0x160c, localization 0x13, font 0x803209a8, skin 0x3a471), original ASSIGN, original 198-row scrolling list and a Drum-style lower color panel.
- Added seven screen colors: Gold, Red, Blue, Purple, Cyan, Green, Orange. Original Cyan/Purple values are unchanged. A/B colors persist in unused bits 8..10 of their existing tagged preset halfwords; legacy defaults migrate without changing preset numbers. BACK cancels both pending values; ASSIGN commits both.
- Candidate row painting temporarily supplies native Drum theme colors (17 background / 24 text) and restores the original properties without invalidating during paint. Scrolling/hit testing remain native. Footer swatches use native button input and a dedicated painter for actual palette fills, white selection border and a checkmark.
- BUTTON COLOR uses the same seven names, keeps old Cyan/Purple codes and maps retired White to Cyan. Gold and Orange currently share the same physical red+green LED mask; their screen colors differ. PWM/channel-brightness control was not added.
- Verified the input/tuner/disabled physical hold implementation remains byte-for-byte identical in C source to r4.
- Host checks include all 131072 tagged words, 39204 preset pairs, 2772 color/preset combinations, transactions, restoration, draw property restoration, seven swatch colors/checkmarks and existing input regressions. ARM checks cover LED masks, storage validation, menu/renderer bounds, exact stock template reuse and inverse container reconstruction.
- Hardware verification of this revision is pending.

## Revision 4 — 2026-11-18

- Disabled individual physical footswitch long actions in Mode 3. Both the long event and following release are swallowed; the next short press works normally. Screen-tile holds for assignment and the native combined-switch tuner action remain available.
- Removed synthetic combined-press inference from accumulated individual pressed bits. Native kind-5 events remain authoritative; a missing individual release can no longer turn a later single-switch down into a false combined hold.
- The exact device freeze was not reproduced without pedal event tracing. The prior long-event path called native_load and the pressed-bit heuristic could suppress another switch after an interrupted stream; both paths are removed for long actions/false combos.
- Regression tests cover both switches, 1–100 repeated long events, following short presses on both switches, an interrupted stream and recovery after native combined actions. Software checks passed; hardware confirmation pending.
- Color selection, Back/header changes and the seven-color BUTTON COLOR palette are design proposals only in this revision. Firmware UI remains as in revision 3.

## Device feedback revision 3 — 2026-11-17

- Expanded preset number labels from 90×36 to 160×46 for the native 42 px font, retaining their right edges and placement above A/B.
- Replaced the Mode 3 parent WM_PAINT path with its own background and the original OUT/IN frame/divider only. Hidden stock widgets alone did not suppress direct native frame painting, leaving the empty frame to the right of OUT/IN and horizontal lines in the A/B gap.
- Mode 1/2 painting and all input/tuner/touch logic remain identical to revision 2.
- Host regressions verify number widths with native font advances, the Mode 3 paint bypass, chooser background, preserved meter frame and native painting fallback for Mode 1/2.
- Software checks pass; revision 3 appearance still needs device confirmation.

## Device feedback revision 2 — 2026-11-17

- Corrected the inverted native list color APIs: 0x25d02 controls three backgrounds, 0x25fac controls five text colors. Configure black backgrounds and white text.
- Corrected native text alignment: horizontal center=2, vertical center=12. Chooser title uses 0x0e in the centered 440×80 header rectangle.
- Increased A/B from the stock 80 px font to the stock 100 px font and enlarged their rectangles.
- Corrected individual key states against stock dispatch: 1=down, 2=long action, 4=release. Short releases now select A/B; long selection remains available, with the final release suppressed. Combined-switch dispatch for tuner is preserved.
- Added instruction-level regressions for actual stock color setters and key-state branches, plus host regressions for short and long sequences, title flags and larger letter fonts. Fixed shifted-register arithmetic in the limited instruction simulator exposed by these regressions.
- New revision still needs confirmation on the pedal. User photos of the preceding revision showed incorrect chooser colors/alignment and inactive short presses; the user confirmed tuner and bottom-left name wrapping worked.

## UI fix — 2026-11-17

- Mode 3 chooser: black list background and white text in all list states; title centered between the native top buttons.
- Native short-release events select only the assigned A/B presets without requiring a separate press event; individual holds remain suppressed; combined presses retain native actions, including tuner, without switching A/B on the subsequent individual releases.
- Bottom-left preset names wrap by native glyph widths into two lines; exceptionally wide names use the smaller native font. Preset numbers sit above A/B.
- Host regressions cover release-only streams, holds, list colors, title alignment and every printable ASCII character repeated across a 16-character name. Native font metrics are checked against exact stock V2.2.
- Hardware rendering, button event streams and audio behavior still require device verification.

# 1.1.0-colours — 2026-11-16

- Local derivative of Pitik upstream 862ad33ec26cc8c579be9697a5e764df3894717a.
- Teal / white / pink build-time selection; pink uses red + blue.
- Exact Hotone V2.2 and verified local Dubshift 1.1 profiles.
- LED-only changes; no About hook, relocation or inserted code.
- SHA-256 gates, independent output manifest, byte-difference and overwrite checks.
- Russian offline UI and preset backup guide. Not tested on hardware.

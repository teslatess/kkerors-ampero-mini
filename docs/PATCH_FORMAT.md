# Patch format, schema 1

The manifest is generated from the existing custom V2.2 source, never from a generic binary diff.
Input and output are pinned by SHA-256 and file size. Offsets are zero-based byte offsets.

- `payload_base64`: compiled extension code and zero-filled stock-template placeholders.
- `insertion`: position at which the reconstructed extension is inserted.
- `copies`: source offsets in the user's unchanged stock input, destination offsets in the extension,
  lengths and explicit small record edits. The six copied regions total 1120 bytes:
  the 980-byte home template, three 28-byte chooser records and two 28-byte footer records.
- `writes`: fixed-length hook/data replacements before insertion; original bytes are checked
  by SHA-256 rather than stored as a firmware fragment in this manifest.
- `header_add_u32`: the six fixed little-endian header words increased by extension length.
- Container length at offset 4 and byte checksum at offset 14 are recomputed.

The reference interpreter is `tools/generate_patch.py:apply`. It verifies the final output
hash before returning bytes. This schema is a recipe for the exact supported stock file,
not a general-purpose firmware patch format. A future browser interpreter must use the
same checks and must not accept a downloaded manifest's hash as an independent trust anchor:
it must pin the supported release hashes in its own reviewed code.

## Local preparation

From the repository root, with an output manifest path that does not exist:

```sh
python3 tools/generate_patch.py '/path/to/official-V2.2.bin' '/path/to/new-V2.2.json'
python3 tests/check_patch.py '/path/to/official-V2.2.bin' '/path/to/local-kkerors-mod.bin'
```

Compare the generated manifest with `docs/patches/V2.2.json` before publishing.
The generator rejects compiler/source output that differs from the pinned reference hash.
No stock or complete modified firmware is needed in the public repository or public CI.

`base_firmware: "V2.2"` identifies the official firmware base, not a separate mod version. The generated filename is `V2.2_kkerors-mod.bin`. No firmware bytes changed during the naming update.

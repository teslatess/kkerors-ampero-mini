#!/usr/bin/env python3
"""Generate a source-derived patch; never serialize stock UI records.
Usage: python3 tools/generate_patch.py STOCK.bin NEW_PATCH.json
"""
import base64
import hashlib
import importlib.util
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('firmware_build', ROOT / 'source/menu/build.py')
builder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(builder)
BASELINE = 'de0fd0b6e61b626939da3ddacd42922cf1e5fdc70131d97fb5e101b09b62e488'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def generate(stock):
    # The existing builder validates the exact stock SHA and inverse reconstruction.
    final, report = builder.build(stock)
    if digest(final) != BASELINE:
        raise ValueError('Source/compiler output differs from v1.0; do not relabel it as v1.0')
    raw, symbols = builder.compile_source()
    payload = bytearray(raw)
    copies = []

    def copy(symbol, delta, source, length, edits=None):
        target = symbols[symbol] - builder.BASE + delta
        if payload[target:target + length] != bytes(length):
            raise ValueError('Expected an empty source-template placeholder')
        copies.append(dict(source=source, target=target, length=length, edits=edits or []))

    copy('home_template', 0, 0x4c390 + 0x48c, 980)
    for i, (source, ident, rect) in enumerate([
        (0x48c + 25 * 28, 0x85b, (20, 92, 760, 276)),
        (0x160c, 0x85c, (20, 0, 146, 80)),
        (0x860 + 56, 0x85d, (633, 0, 146, 80)),
    ]):
        copy('chooser_controls', i * 28, 0x4c390 + source, 28,
             [dict(offset=8, hex=struct.pack('<5H', ident, *rect).hex())])
    for i, source in enumerate((0x1660, 0x16d0)):
        copy('footer_controls', i * 28, 0x4c390 + source, 28,
             [dict(offset=8, hex=struct.pack('<H', 0x85e + i).hex())])
    w = struct.unpack_from('<8I', stock, 64)
    insertion = 80 + w[4] - 0x80000 + w[5]
    writes = []
    for hook in report['hooks']:
        old = bytes.fromhex(hook['before'])
        writes.append(dict(offset=80 + w[4] - 0x80000 + int(hook['address'], 16),
                           length=len(old), expected_sha256=digest(old), hex=hook['after']))
    patch = dict(format='kkerors-mod-patch', schema=1, release='v1.0',
                 input=dict(size=len(stock), sha256=builder.SHA),
                 output=dict(size=len(final), sha256=BASELINE, filename='V2.2_kkerors-mod-v1.0.bin'),
                 insertion=insertion, payload_base64=base64.b64encode(payload).decode(),
                 copies=copies, writes=writes,
                 header_add_u32=[52, 56, 64, 72, 84, 88])
    if apply(stock, patch) != final:
        raise ValueError('Independent patch reconstruction differs from builder')
    return patch


def apply(stock, patch):
    """Reference interpreter; browser implementation will follow the same schema."""
    def span(offset, length, size):
        if type(offset) is not int or type(length) is not int or offset < 0 or length < 0 or offset + length > size:
            raise ValueError('Operation out of bounds')

    if patch['format'] != 'kkerors-mod-patch' or patch['schema'] != 1:
        raise ValueError('Unsupported patch format')
    if len(stock) != patch['input']['size'] or digest(stock) != patch['input']['sha256']:
        raise ValueError('Wrong input firmware')
    payload = bytearray(base64.b64decode(patch['payload_base64'], validate=True))
    used = set()
    for op in patch['copies']:
        s, t, n = op['source'], op['target'], op['length']
        span(s, n, len(stock)); span(t, n, len(payload))
        region = set(range(t, t + n))
        if used & region or payload[t:t + n] != bytes(n):
            raise ValueError('Conflicting copy operation')
        used |= region
        payload[t:t + n] = stock[s:s + n]
        for edit in op['edits']:
            data = bytes.fromhex(edit['hex'])
            span(edit['offset'], len(data), n)
            at = t + edit['offset']
            payload[at:at + len(data)] = data
    ins = patch['insertion']; span(ins, 0, len(stock))
    out = bytearray(stock[:ins] + payload + stock[ins:])
    written = set()
    for op in patch['writes']:
        at, n = op['offset'], op['length']; span(at, n, ins)
        data = bytes.fromhex(op['hex'])
        region = set(range(at, at + n))
        if region & written or len(data) != n or digest(stock[at:at + n]) != op['expected_sha256']:
            raise ValueError('Invalid write precondition')
        written |= region
        out[at:at + n] = data
    if patch['header_add_u32'] != [52, 56, 64, 72, 84, 88]:
        raise ValueError('Unsupported header operations')
    for at in patch['header_add_u32']:
        value = struct.unpack_from('<I', stock, at)[0] + len(payload)
        struct.pack_into('<I', out, at, value)
    struct.pack_into('<I', out, 4, len(out) - 48)
    out[14] = sum(out[48:]) % 256
    if len(out) != patch['output']['size'] or digest(out) != patch['output']['sha256']:
        raise ValueError('Output verification failed')
    return bytes(out)


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    stock = Path(sys.argv[1]).read_bytes()
    patch = generate(stock)
    with Path(sys.argv[2]).open('x') as f:
        json.dump(patch, f, indent=2); f.write('\n')
    print('Verified v1.0 patch:', patch['output']['sha256'])

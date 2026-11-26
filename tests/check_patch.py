#!/usr/bin/env python3
"""Local integration checks. Firmware inputs remain outside the public repository.
Usage: python3 tests/check_patch.py STOCK.bin BASELINE.bin
"""
import base64
import copy
import importlib.util
import json
import sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('generate_patch', ROOT / 'tools/generate_patch.py')
gen = importlib.util.module_from_spec(spec); spec.loader.exec_module(gen)


def main():
    stock, baseline = [Path(p).read_bytes() for p in sys.argv[1:]]
    patch = json.loads((ROOT / 'docs/patches/v1.0.json').read_text())
    original = bytes(stock)
    assert gen.generate(stock) == patch, 'Patch generation is not reproducible'
    assert gen.apply(stock, patch) == baseline, 'Not byte-identical to saved baseline'
    assert stock == original, 'Input modified'
    raw = base64.b64decode(patch['payload_base64'])
    assert len(patch['copies']) == 6
    for op in patch['copies']:
        assert raw[op['target']:op['target'] + op['length']] == bytes(op['length'])
        # No complete stock UI record/template is embedded in the added code.
        assert stock[op['source']:op['source'] + op['length']] not in raw

    rejected = 0
    def reject(data, manifest):
        nonlocal rejected
        try:
            gen.apply(data, manifest)
        except (ValueError, KeyError, OverflowError):
            rejected += 1
        else:
            raise AssertionError('Invalid input accepted')

    reject(stock[:-1], patch)
    damaged = bytearray(stock); damaged[-1] ^= 1; reject(damaged, patch)
    reject(baseline, patch)
    mutations = [
        lambda p: p.update(schema=2),
        lambda p: p.update(insertion=len(stock) + 1),
        lambda p: p['copies'][0].update(source=len(stock)),
        lambda p: p['copies'][0].update(target=-1),
        lambda p: p['copies'][0]['edits'].append(dict(offset=980, hex='ff')),
        lambda p: p['copies'].append(copy.deepcopy(p['copies'][0])),
        lambda p: p['writes'][0].update(expected_sha256='0' * 64),
        lambda p: p['writes'][0].update(offset=p['insertion']),
        lambda p: p['writes'][0].update(hex=''),
        lambda p: p['writes'].append(copy.deepcopy(p['writes'][0])),
        lambda p: p['header_add_u32'].append(96),
        lambda p: p['output'].update(sha256='0' * 64),
        lambda p: p['output'].update(size=1),
        lambda p: p.update(payload_base64='!'),
    ]
    for mutation in mutations:
        p = copy.deepcopy(patch); mutation(p); reject(stock, p)
    print(f'PASS: exact baseline, reproducible manifest, immutable input, six empty template regions, {rejected} rejection cases')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    main()

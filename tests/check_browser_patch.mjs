import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {applyPatch, RELEASE, sha256} from '../docs/patcher.js';
const patch=JSON.parse(await readFile(new URL('../docs/patches/v1.0.json',import.meta.url),'utf8'));
assert.equal(patch.input.sha256,RELEASE.inputHash);assert.equal(patch.output.sha256,RELEASE.outputHash);
assert.equal(patch.output.filename,RELEASE.filename);
await assert.rejects(applyPatch(new Uint8Array(2),patch));
await assert.rejects(applyPatch(new Uint8Array(RELEASE.inputSize),patch));
if(process.argv.length===2){console.log('PASS public checks: release metadata and invalid input rejection. Real firmware integration not run.');process.exit(0);}
assert.equal(process.argv.length,4,'Pass STOCK.bin BASELINE.bin');
const input=new Uint8Array(await readFile(process.argv[2])), baseline=new Uint8Array(await readFile(process.argv[3]));
const before=await sha256(input), output=await applyPatch(input,patch);
assert.deepEqual(output,baseline);assert.equal(await sha256(input),before);
await assert.rejects(applyPatch(output,patch));
const cases=[
 p=>p.input.sha256='0'.repeat(64),p=>p.output.sha256='0'.repeat(64),p=>p.output.filename='evil.bin',
 p=>p.schema=2,p=>p.payload_base64='!',p=>p.insertion=input.length+1,
 p=>p.copies[0].source=input.length,p=>p.copies[0].target=-1,
 p=>p.copies[0].edits.push({offset:980,hex:'ff'}),p=>p.copies[1].target=p.copies[0].target,
 p=>p.writes[0].expected_sha256='0'.repeat(64),p=>p.writes[0].hex='z0',
 p=>p.writes[0].offset=p.insertion,p=>p.writes[1].offset=p.writes[0].offset,
 p=>p.header_add_u32.push(96),p=>p.copies=[],p=>p.writes=[],
 p=>p.writes[0].hex='00'.repeat(p.writes[0].length),
];
for(const change of cases){const p=structuredClone(patch);change(p);await assert.rejects(applyPatch(input,p));}
console.log(`PASS browser engine: byte-identical v1.0, immutable input, repeat-patching rejection, ${cases.length} manifest rejection cases.`);

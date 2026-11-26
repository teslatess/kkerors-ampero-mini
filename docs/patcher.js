/* MIT; see ../LICENSE and ../THIRD_PARTY_NOTICES.md. */
export const RELEASE = Object.freeze({version:'v1.0', inputSize:7598420, outputSize:7604952,
  inputHash:'2928e3ca297cce1759944418cb91ce73a936a6b35032219086b652c3708c6f7c',
  outputHash:'de0fd0b6e61b626939da3ddacd42922cf1e5fdc70131d97fb5e101b09b62e488',
  filename:'V2.2_kkerors-mod-v1.0.bin'});
export async function sha256(bytes) {
  return Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',bytes)),x=>x.toString(16).padStart(2,'0')).join('');
}
export async function validateInput(input) {
  if (!(input instanceof Uint8Array) || input.length !== RELEASE.inputSize)
    throw new Error('An original Hotone Ampero Mini V2.2 .bin is required. The file size does not match.');
  if (await sha256(input) !== RELEASE.inputHash)
    throw new Error('The file does not match stock V2.2. Modified or corrupted firmware is not supported.');
}
function span(at,n,size) {
  if (!Number.isSafeInteger(at)||!Number.isSafeInteger(n)||at<0||n<0||at>size-n) throw new Error('A patch operation is out of bounds.');
}
function hex(text) {
  if(typeof text!=='string'||text.length%2||!/^[0-9a-f]*$/.test(text)) throw new Error('Invalid patch data.');
  return Uint8Array.from(text.match(/../g)||[],x=>parseInt(x,16));
}
function claim(regions,at,n) {
  if(regions.some(([a,b])=>at<b&&at+n>a)) throw new Error('Overlapping patch operations.');
  regions.push([at,at+n]);
}
export async function applyPatch(input,p) {
  await validateInput(input);
  if(p.format!=='kkerors-mod-patch'||p.schema!==1||p.release!==RELEASE.version||
     p.input?.size!==RELEASE.inputSize||p.input?.sha256!==RELEASE.inputHash||
     p.output?.size!==RELEASE.outputSize||p.output?.sha256!==RELEASE.outputHash||p.output?.filename!==RELEASE.filename)
    throw new Error('The patch does not match the supported release.');
  if(typeof p.payload_base64!=='string'||p.payload_base64.length>20000||!Array.isArray(p.copies)||p.copies.length!==6||
     !Array.isArray(p.writes)||p.writes.length!==14) throw new Error('Invalid patch structure.');
  const payload=Uint8Array.from(atob(p.payload_base64),c=>c.charCodeAt(0));
  if(payload.length!==RELEASE.outputSize-RELEASE.inputSize) throw new Error('Invalid extension length.');
  const regions=[];
  for(const op of p.copies) {
    span(op.source,op.length,input.length);span(op.target,op.length,payload.length);claim(regions,op.target,op.length);
    if(payload.subarray(op.target,op.target+op.length).some(x=>x!==0)||!Array.isArray(op.edits)||op.edits.length>5)
      throw new Error('Invalid patch template.');
    payload.set(input.subarray(op.source,op.source+op.length),op.target);
    for(const edit of op.edits) {const bytes=hex(edit.hex);span(edit.offset,bytes.length,op.length);payload.set(bytes,op.target+edit.offset);}
  }
  span(p.insertion,0,input.length);
  const out=new Uint8Array(RELEASE.outputSize);
  out.set(input.subarray(0,p.insertion));out.set(payload,p.insertion);out.set(input.subarray(p.insertion),p.insertion+payload.length);
  const writes=[];
  for(const op of p.writes) {
    span(op.offset,op.length,p.insertion);claim(writes,op.offset,op.length);
    const bytes=hex(op.hex);
    if(bytes.length!==op.length||await sha256(input.subarray(op.offset,op.offset+op.length))!==op.expected_sha256)
      throw new Error('The original data does not match the patch precondition.');
    out.set(bytes,op.offset);
  }
  if(JSON.stringify(p.header_add_u32)!==JSON.stringify([52,56,64,72,84,88])) throw new Error('Invalid header operations.');
  const source=new DataView(input.buffer,input.byteOffset,input.byteLength), target=new DataView(out.buffer);
  for(const at of p.header_add_u32) {const value=source.getUint32(at,true)+payload.length;if(value>0xffffffff)throw new Error('Header value overflow.');target.setUint32(at,value,true);}
  target.setUint32(4,out.length-48,true);
  let sum=0;for(let i=48;i<out.length;i++)sum=(sum+out[i])&255;out[14]=sum;
  if(await sha256(out)!==RELEASE.outputHash)throw new Error('The output checksum does not match. Download cancelled.');
  return out;
}

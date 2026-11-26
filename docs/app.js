import {RELEASE, validateInput, applyPatch} from './patcher.js';
const file=document.querySelector('#firmware'), build=document.querySelector('#build'), status=document.querySelector('#status'), download=document.querySelector('#download');
let selected=null, generation=0, url=null;
function clearDownload(){download.hidden=true;download.removeAttribute('href');if(url){URL.revokeObjectURL(url);url=null;}}
function message(text,state=''){status.textContent=text;status.dataset.state=state;}
file.addEventListener('change',async()=>{
  const token=++generation;selected=null;build.disabled=true;clearDownload();
  const choice=file.files[0];if(!choice){message('Choose the original firmware file.');return;}
  if(choice.size!==RELEASE.inputSize){message('The file size does not match stock V2.2. Choose the extracted official .bin file.','error');return;}
  message('Checking the original firmware…');
  try {const bytes=new Uint8Array(await choice.arrayBuffer());await validateInput(bytes);if(token!==generation)return;selected=bytes;build.disabled=false;message('Original V2.2 verified. Ready to build v1.0.','success');}
  catch(e){if(token===generation)message(e.message,'error');}
});
build.addEventListener('click',async()=>{
  if(!selected)return;build.disabled=true;file.disabled=true;clearDownload();message('Building firmware and verifying the result…');
  try {
    const response=await fetch('./patches/v1.0.json',{cache:'no-store'});if(!response.ok)throw new Error('Could not load the patch. Refresh the page and try again.');
    const bytes=await applyPatch(selected,await response.json());
    url=URL.createObjectURL(new Blob([bytes],{type:'application/octet-stream'}));download.href=url;download.download=RELEASE.filename;download.hidden=false;
    message('Done. The output SHA-256 matches the reference v1.0 build.','success');
  }catch(e){message(e.message,'error');}
  finally {file.disabled=false;build.disabled=!selected;}
});
if(!globalThis.crypto?.subtle){file.disabled=true;message('Open this page over HTTPS or a localhost server to enable file verification.','error');}
window.addEventListener('pagehide',clearDownload);

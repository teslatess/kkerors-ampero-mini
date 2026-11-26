#!/usr/bin/env python3
"""Build experimental stock V2.2 firmware with Mode 3 A/B and BUTTON COLOR.
Usage: python3 menu/build.py <original stock V2.2.bin> <NEW output.bin>
Requires clang with ARM assembler; reads the firmware without modifying it.
"""
import hashlib,json,struct,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parent
SHA='2928e3ca297cce1759944418cb91ce73a936a6b35032219086b652c3708c6f7c'
BASE=0x4c324

def branch(site,target,link=False):
 off=(target&~1)-(site+4)
 if off&1 or not -(1<<24)<=off<(1<<24):raise ValueError('branch out of range')
 s,i1,i2=(off>>24)&1,(off>>23)&1,(off>>22)&1
 a=0xf000|(s<<10)|((off>>12)&0x3ff)
 z=(0xd000 if link else 0x9000)|(((1-i1)^s)<<13)|(((1-i2)^s)<<11)|((off>>1)&0x7ff)
 return struct.pack('<HH',a,z)

def compile_source():
 with tempfile.TemporaryDirectory() as tmp:
  obj=Path(tmp)/'code.o'
  c_asm=Path(tmp)/'mode3.s'
  subprocess.run(['clang','-target','armv7em-none-eabi','-mcpu=cortex-m7','-mthumb','-mfloat-abi=soft','-mno-movt','-Oz','-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-S',str(ROOT/'mode3.c'),'-o',str(c_asm)],check=True)
  import re
  generated=re.sub(r'^\s*\.section[^\n]*$', '.text', c_asm.read_text(), flags=re.M)
  source=Path(tmp)/'all.s'
  source.write_text((ROOT/'button_color.s').read_text()+'\n'+(ROOT/'mode3_bridge.s').read_text()+'\n'+generated)
  subprocess.run(['clang','-target','armv7em-none-eabi','-mcpu=cortex-m7','-c',str(source),'-o',str(obj)],check=True)
  e=obj.read_bytes()
 # ELF32 little endian relocatable object; .text is laid out at its actual ITCM addresses with .org.
 if e[:7]!=b'\x7fELF\x01\x01\x01':raise ValueError('unexpected ELF')
 shoff=struct.unpack_from('<I',e,32)[0];ents,n,strings=struct.unpack_from('<HHH',e,46)
 sections=[struct.unpack_from('<10I',e,shoff+i*ents) for i in range(n)]
 shstr=sections[strings];names=e[shstr[4]:shstr[4]+shstr[5]]
 def name(at,table):return table[at:table.index(b'\0',at)].decode()
 named={name(s[0],names):i for i,s in enumerate(sections)}
 ti=named['.text'];t=sections[ti];code=bytearray(e[t[4]:t[4]+t[5]])
 symbols=[];symbol_names={}
 for sec in sections:
  if sec[1]!=2:continue
  st=sections[sec[6]];strtab=e[st[4]:st[4]+st[5]]
  for at in range(sec[4],sec[4]+sec[5],sec[9]):
   no,v,size,info,other,section=struct.unpack_from('<IIIBBH',e,at)
   nm=name(no,strtab);symbols.append((nm,v,section));symbol_names[nm]=v
 for sec in sections:
  if sec[1]!=9 or sec[7]!=ti:continue
  for at in range(sec[4],sec[4]+sec[5],8):
   address,info=struct.unpack_from('<II',e,at);kind=info&255;sym=symbols[info>>8]
   if kind in (10,30) and sym[2]==ti:
    code[address:address+4]=branch(address,sym[1],kind==10)
   elif kind==2 and sym[2]==ti:
    addend=struct.unpack_from('<I',code,address)[0]
    struct.pack_into('<I',code,address,(addend+sym[1])&0xffffffff)
   else:raise ValueError('unsupported relocation: '+str((address,kind,sym)))
 if any(code[:BASE]):raise ValueError('unexpected bytes before patch region')
 code+=bytes(-len(code)%4)
 return bytes(code[BASE:]),symbol_names

def build(stock):
 if hashlib.sha256(stock).hexdigest()!=SHA:raise ValueError('Only exact original Hotone Mini V2.2 accepted')
 w=struct.unpack_from('<8I',stock,64)
 payload,symbols=compile_source();payload=bytearray(payload)
 # Copy original 35-record main-window template into the appended segment and add labels, native list, Drum BACK, SAVE and native Drum color label/selector.
 template=symbols['home_template']-BASE
 original_template=stock[0x4c390+0x48c:0x4c390+0x48c+980]
 if len(original_template)!=980:raise ValueError('template length')
 payload[template:template+980]=original_template
 # Reuse stock list, actual Drum BACK and CTRL/EXP button records.
 controls=symbols['chooser_controls']-BASE
 records=[]
 for source,ident,rect in [(0x48c+25*28,0x85b,(20,92,760,276)),
                            (0x160c,0x85c,(20,0,146,80)),
                            (0x860+56,0x85d,(633,0,146,80))]:
  record=bytearray(stock[0x4c390+source:0x4c390+source+28])
  struct.pack_into('<5H',record,8,ident,*rect);records.append(record)
 payload[controls:controls+84]=b''.join(records)
 if controls!=template+46*28:raise ValueError('noncontiguous widget template')
 footer=symbols['footer_controls']-BASE
 if footer!=template+49*28:raise ValueError('noncontiguous footer')
 for i,source in enumerate((0x1660,0x16d0)):
  record=bytearray(stock[0x4c390+source:0x4c390+source+28])
  struct.pack_into('<H',record,8,0x85e+i)
  payload[footer+i*28:footer+(i+1)*28]=record
 n=len(payload)
 if w[5]!=BASE or BASE+n>0x80000:raise ValueError('ITCM bounds')
 ins=80+w[4]-0x80000+w[5]
 out=bytearray(stock[:ins]+payload+stock[ins:])
 edits=[]
 def edit(site,old,new):
  off=80+w[4]-0x80000+site
  if stock[off:off+len(old)]!=old:raise ValueError('unexpected hook site '+hex(site))
  out[off:off+len(new)]=new;edits.append(dict(address=hex(site),before=old.hex(),after=new.hex()))
 # Dedicated hooks, with verified instruction boundaries.
 for site,old,symbol,link in [
  (0x167d8,'70b5d94e','event_hook',False),
  (0x1781c,'286b40f61101','display_hook',False),
  (0x28204,'2de9f003','led_hook',False),
  (0x16dc4,'a4f8f060','validate_reserved',False),
  (0x7658,'0ff074fa','restore_reserved',True),
  (0x1fc54,'70b50446','mode3_input',False),
  (0x1f230,'2de9f04f','mode3_update',False),
  (0x1793a,'bdf89800e8b3','mode_name_hook',False)]:
  original=bytes.fromhex(old);patch=branch(site,symbols[symbol],link)
  if len(original)==6:patch+=bytes.fromhex('00bf')
  edit(site,original,patch)
 # Exact original instructions/data are checked before every edit.
 for site,old,new in [(0x28584,'0128','0228'),(0x16d8c,'0128','0228'),
                      (0x17926,'0122','0222'),(0x1fc2e,'2321','3321')]:
  edit(site,bytes.fromhex(old),bytes.fromhex(new))
 edit(0x1fc4c,struct.pack('<I',0x46929),struct.pack('<I',symbols['mode3_callback']))
 edit(0x1fc50,struct.pack('<I',0x8000048c),struct.pack('<I',symbols['home_template']))
 for i in (0,2,5,6):struct.pack_into('<I',out,64+4*i,w[i]+n)
 for a in (52,56):struct.pack_into('<I',out,a,struct.unpack_from('<I',out,a)[0]+n)
 struct.pack_into('<I',out,4,len(out)-48);out[14]=sum(out[48:])%256
 # Full inverse reconstruction: no original firmware bytes may change outside listed hooks/header fields.
 reverted=bytearray(out[:ins]+out[ins+n:])
 for e in edits:
  o=80+w[4]-0x80000+int(e['address'],16);v=bytes.fromhex(e['before']);reverted[o:o+len(v)]=v
 for a,length in [(4,4),(14,1),(52,8),(64,32)]:reverted[a:a+length]=stock[a:a+length]
 if reverted!=stock:raise ValueError('unexpected modification outside hooks')
 report=dict(input_sha256=SHA,output_sha256=hashlib.sha256(out).hexdigest(),extra_bytes=n,hooks=edits,
  symbols={k:hex(v&~1) for k,v in symbols.items() if k in ('choice','event_hook','display_hook','led_hook','restore_reserved','validate_reserved','mode3_input','mode3_update','mode3_callback','home_template','mode3_state')},
  hardware_tested=False,storage='global +0xf0: BC00..BC07 LED code (legacy preserved); +0xf2/+0xf4: A/B high tag, bits 8..10 color+1 (0 legacy default), low byte preset 0..197')
 return bytes(out),report

def main():
 if len(sys.argv)!=3:sys.exit(__doc__)
 source,dest=map(Path,sys.argv[1:]);out,report=build(source.read_bytes())
 with dest.open('xb') as f:f.write(out)
 print(json.dumps(report,indent=2))
if __name__=='__main__':main()

#!/usr/bin/env python3
"""Instruction-level simulation of added Thumb hooks using objdump decoding.
Stock UI/audio/flash functions are stubs. This is NOT full device emulation.
Usage: python3 menu/check.py <original Hotone V2.2.bin>
"""
import re,struct,subprocess,sys,tempfile
from pathlib import Path
from build import build,compile_source,BASE
stock=Path(sys.argv[1]).read_bytes();out,report=build(stock);payload,symbols=compile_source()
itcm=out[0x6c:0x6c+BASE+len(payload)]
with tempfile.TemporaryDirectory() as tmp:
 p=Path(tmp);(p/'code.bin').write_bytes(itcm)
 (p/'code.s').write_text('.syntax unified\n.thumb\n.section .text\n.thumb_func\ncode:\n.incbin "'+str(p/'code.bin')+'"\n')
 subprocess.run(['clang','-target','armv7em-none-eabi','-c',str(p/'code.s'),'-o',str(p/'code.o')],check=True)
 listing=subprocess.check_output(['objdump','-d','--triple=thumbv7em-none-eabi',str(p/'code.o')],text=True)
# Pinned GUI_FONT/GUI_FONT_PROP/GUI_CHARINFO layouts used for pixel wrapping.
w=struct.unpack_from('<8I',stock,64)
def flash(address,size):
 offset=80+w[6]-0x80000+address-0x80000000
 return stock[offset:offset+size]
for font,prop,chars,fixture in ((0x801f5ff4,0x801f5fe8,0x801f4f24,'widths'),
                                (0x80162934,0x80162928,0x80161864,'smallWidths')):
 assert struct.unpack_from('<I',flash(font,32),28)[0]==prop
 first,last,table=struct.unpack_from('<HHI',flash(prop,8))
 assert (first,last,table)==(32,126,chars)
 expected=[flash(chars+i*8,2)[1] for i in range(95)]
 test_source=(Path(__file__).parent/'mode3_test.c').read_text()
 actual=list(map(int,re.search(r'\b'+fixture+r'\[95\]=\{([^}]+)\}',test_source)[1].split(',')))
 assert actual==expected
print('Native font metrics and host wrapping fixtures match stock V2.2')
instructions={}
for line in listing.splitlines():
 m=re.match(r'\s*([0-9a-f]+):\s+([0-9a-f]{4}(?: [0-9a-f]{4})?)\s+([^\s]+)\s*(.*)',line)
 if m:instructions[int(m[1],16)]=(len(m[2].replace(' ',''))//2,m[3],m[4].split('@')[0].strip())

class CPU:
 def __init__(self):
  self.r=[0]*16;self.r[13]=0x2003f000;self.r[14]=0x70001
  self.mem={i:v for i,v in enumerate(itcm)};self.z=False;self.n=False;self.c=False;self.v=False;self.calls=[];self.it=[];self.stubs={symbols['mode3_menu']&~1:lambda c:0};self.stop=None
 def write(self,addr,value,size=4):
  for i in range(size):self.mem[addr+i]=(value>>(8*i))&255
 def read(self,addr,size=4):return sum(self.mem.get(addr+i,0)<<(8*i) for i in range(size))
 def string(self,addr):
  result=bytearray()
  while self.read(addr,1) and len(result)<100:result.append(self.read(addr,1));addr+=1
  return result.decode()
 def reg(self,s):return {'sp':13,'lr':14,'pc':15}.get(s, int(s[1:]) if s.startswith('r') else -1)
 def value(self,s):return int(s[1:],0) if s.startswith('#') else self.r[self.reg(s)]
 def condition(self,s):return {'eq':self.z,'ne':not self.z,'hi':self.c and not self.z,'hs':self.c,'lo':not self.c,'ls':not self.c or self.z}[s]
 def address(self,s,pc):
  terms=s.strip('[]').split(',');base=terms[0].strip();a=((pc+4)&~3) if base=='pc' else self.value(base)
  if len(terms)>1:a+=self.value(terms[1].strip()) << (int(terms[2].strip().split('#')[1],0) if len(terms)>2 else 0)
  return a
 def run(self,address,stops=()):
  pc=address&~1
  for steps in range(5000):
   if pc in stops or pc==0x70000:self.stop=pc;return
   if pc in self.stubs:
    self.calls.append((pc,tuple(self.r[:4])))
    result=self.stubs[pc](self)
    if result is not None:self.r[0]=result&0xffffffff
    pc=self.r[14]&~1;continue
   if pc not in instructions:raise AssertionError('Unknown PC '+hex(pc))
   size,op,args=instructions[pc];nxt=pc+size
   cond=self.it.pop(0) if self.it else None
   if cond is not None and not cond:pc=nxt;continue
   op=op.removesuffix('.w');parts=[a.strip() for a in args.split(',')]
   if op.startswith('it'):
    current=self.condition(args);self.it=[current]+[current if x=='t' else not current for x in op[2:]]
   else:
    # objdump prints condition suffixes on instructions governed by IT.
    for suffix in ('eq','ne','hi','hs','lo','ls'):
     if op.endswith(suffix) and op not in ('beq','bne','bhi','bhs','blo','bls'):
      op=op[:-len(suffix)];break
    if op in ('mov','movs','movw'):
     self.r[self.reg(parts[0])]=self.value(parts[1])&0xffffffff
    elif op in ('ldr','ldrh','ldrb'):
     addr=self.address(args[args.index('['):],pc)
     self.r[self.reg(parts[0])]=self.read(addr,{'ldr':4,'ldrh':2,'ldrb':1}[op])
    elif op in ('str','strh','strb'):
     addr=self.address(args[args.index('['):],pc)
     self.write(addr,self.value(parts[0]),{'str':4,'strh':2,'strb':1}[op])
    elif op in ('cmp','subs','sub','adds','add','orrs','and','ands'):
     if op=='cmp':a,b=self.value(parts[0]),self.value(parts[1])
     elif len(parts)==2:a,b=self.value(parts[0]),self.value(parts[1])
     else:
      a,b=self.value(parts[1]),self.value(parts[2])
      if len(parts)>3:
       shift,amount=parts[3].split();amount=int(amount[1:],0)
       if shift=='lsl':b=(b<<amount)&0xffffffff
       elif shift=='lsr':b=b>>amount
       elif shift=='asr':b=((b-0x100000000) if b&0x80000000 else b)>>amount
       else:raise AssertionError('Unsupported shift '+shift)
     v=(a+b) if op.startswith('add') else (a|b) if op=='orrs' else (a&b) if op in ('and','ands') else a-b
     if op in ('cmp','subs','adds','orrs','ands'):
      self.z=(v&0xffffffff)==0;self.n=bool(v&0x80000000);self.c=a>=b if op in ('cmp','subs') else v>0xffffffff
     if op!='cmp':self.r[self.reg(parts[0])]=v&0xffffffff
    elif op=='adr':
     self.r[self.reg(parts[0])]=((pc+4)&~3)+self.value(parts[1].split()[0])
    elif op in ('push','pop'):
     regs=[]
     for item in args.strip('{}').split(','):
      item=item.strip()
      if '-' in item:
       a,b=item.split('-');regs+=list(range(self.reg(a),self.reg(b)+1))
      else:regs.append(self.reg(item))
     regs.sort()
     if op=='push':
      self.r[13]-=4*len(regs)
      for i,r in enumerate(regs):self.write(self.r[13]+4*i,self.r[r])
     else:
      sp=self.r[13]
      for i,r in enumerate(regs):self.r[r]=self.read(sp+4*i)
      self.r[13]+=4*len(regs)
      if 15 in regs:nxt=self.r[15]&~1
    elif op in ('b','bl','bx','blx','beq','bne','bhi','bhs','blo','bls'):
     take=len(op)<=3 or self.condition(op[1:])
     if op in ('beq','bne','bhi','bhs','blo','bls'):take=self.condition(op[1:])
     if take:
      target=int(parts[0].split()[0],0) if parts[0].startswith('0x') else self.value(parts[0])
      if op in ('bl','blx'):self.r[14]=nxt|1
      nxt=target&~1
    elif op!='nop':raise AssertionError('Unsupported '+str((hex(pc),op,args)))
   pc=nxt
  raise AssertionError('Instruction limit')

# Execute the real stock setters instead of trusting the names of mocked APIs.
# The three background slots are +0x54; five text slots are +0x40.
source_text=(Path(__file__).parent/'mode3.c').read_text()
assert 'NATIVE(native_list_bg,0x25d02' in source_text
assert 'NATIVE(native_list_fg,0x25fac' in source_text
for address,count,offset in ((0x25d02,3,0x54),(0x25fac,5,0x40)):
 for state in range(count):
  c=CPU();obj=0x20020000
  for slot in range(0x34,0x60,4):c.write(obj+slot,0x123456)
  for api in (0x10a9c,0xf084,0x11e34,0x3298c):c.stubs[api]=lambda c:None
  c.stubs[0xefcc]=lambda c:obj
  c.r[:3]=[123,state,0xabcdef];c.run(address)
  for slot in range(0x34,0x60,4):assert c.read(obj+slot)==(0xabcdef if slot==offset+state*4 else 0x123456),(hex(address),state,hex(slot),hex(c.read(obj+slot)),[hex(c.read(obj+x)) for x in range(0x34,0x60,4)])
# Actual text painter: horizontal center is 2, vertical center is 12.
assert instructions[0xfb1c][1:] == ('cmp', 'r1, #0x2')
assert instructions[0xfaa6][1:] == ('cmp', 'r1, #0xc')
# Actual individual-key dispatch separates down, long action and release.
for state,stop in ((1,0xd460),(2,0xd428),(4,0xd41a)):
 c=CPU();c.r[4]=0x20020000;c.r[6]=0x20021000;c.write(c.r[4]+6,state,1)
 c.run(0xd404,{stop});assert c.stop==stop
assert flash(0x801da0b0+24,4)==bytes((100,100,1,1))
print('Real native list color setters, text centering flags, 100px font and key-state dispatch OK')

# All LED masks, seven colors, legacy White migration and invalid words.
codes=(0xbc03,0xbc04,0xbc05,0xbc02,0xbc00,0xbc06,0xbc07)
names=('Gold','Red','Blue','Purple','Cyan','Green','Orange')
left_masks=(3,2,4,6,5,1,3)
led_map={code:(mask,mask<<3) for code,mask in zip(codes,left_masks)}
led_map[0xbc01]=(5,40) # Legacy White maps to Cyan; no stale menu index.
for stored in (0,0xffff,0xbc08,*range(0xbc00,0xbc08)):
 for mask in range(64):
  c=CPU();c.write(0x200089d6,stored,2);c.r[:3]=[mask,2,123];sp=c.r[13]
  c.run(0x28204,{0x28208});expect=mask
  if mask in (5,40):expect=led_map.get(stored,(5,40))[mask==40]
  assert c.r[:3]==[expect,2,123]
  assert c.r[13]==sp-24
print('LED hook: all 64 masks, 7 colors, legacy codes and invalid storage OK')

# Reserved field validation preserves exactly three tagged values; neighbours untouched.
for stored in (0,0xffff,0xbbff,*range(0xbc00,0xbc09)):
 c=CPU();c.r[4]=0x20010000;c.r[6]=0;c.write(c.r[4]+0xf0,stored,2);c.write(c.r[4]+0xf2,0x1234,2)
 c.run(symbols['validate_reserved'],{0x16dd0})
 assert c.read(c.r[4]+0xf0,2)==(stored if 0xbc00<=stored<=0xbc07 else 0)
 assert c.read(c.r[4]+0xf2,2)==0
for a in (0,0xffff,0x9fff,0xa800,*[0xa000|(color<<8)|preset for color in range(8) for preset in (0,197,198)]):
 for b in (0,0xffff,0xb800,0xb000,0xb7c5,0xb7c6):
  c=CPU();c.r[4]=0x20010000;c.r[6]=0
  c.write(c.r[4]+0xf2,a,2);c.write(c.r[4]+0xf4,b,2)
  c.run(symbols['validate_reserved'],{0x16dd0})
  assert c.read(c.r[4]+0xf2,2)==(a if (a&0xf800)==0xa000 and (a&255)<198 else 0)
  assert c.read(c.r[4]+0xf4,2)==(b if (b&0xf800)==0xb000 and (b&255)<198 else 0)
print('Reserved field validation OK')
# The restore path calls the stock validator and copies only the tagged reserved halfword.
for stored in (0,0xbc00,0xbc01,0xbc02,0xffff):
 c=CPU();source=0x20011000;c.r[0]=source;c.write(source+0xf0,stored,2)
 def validate(c):
  c.write(c.r[0]+0xf0,stored if 0xbc00<=stored<=0xbc07 else 0,2)
 c.write(source+0xf2,0xa0c5,2);c.write(source+0xf4,0xb063,2)
 c.stubs[0x16b44]=validate
 c.run(symbols['restore_reserved'])
 assert c.read(0x200089d6,2)==(stored if 0xbc00<=stored<=0xbc07 else 0)
 assert c.read(0x200089d8,2)==0xa0c5 and c.read(0x200089da,2)==0xb063
print('Reserved restore copy OK (stock validator stubbed)')


# Real compiled event dispatch; stock timer/LED refresh functions are bounded stubs.
for selected in range(7):
 for action in (0,1,2):
  c=CPU();c.write(0x20005c72,3,1);c.write(0x20005c73,1,1);c.write(0x200089d6,codes[selected],2)
  c.r[:2]=[0,action];c.stubs[0x19638]=lambda c:0;c.run(0x167d8)
  expected=max(0,selected-1) if action==1 else min(6,selected+1) if action==2 else selected
  assert c.read(0x200089d6,2)==codes[expected]
  if expected!=selected:assert c.read(0x200061a0)==10 and c.read(0x20005c75,1)==1
for timer in range(9):
 for action in (0,1,2):
  c=CPU();c.write(0x20005c72,3,1);c.write(0x20005c73,1,1);c.write(0x200089c2,timer,2)
  c.r[:2]=[1,action];c.stubs[0x16670]=lambda c:c.read(0x200089c2,2)
  c.stubs[0x28a68]=lambda c:c.write(0x200089c2,c.r[0],2)
  c.run(0x167d8);expected=max(0,timer-1) if action==1 else min(8,timer+1) if action==2 else timer
  assert c.read(0x200089c2,2)==expected
for section,page in ((0,0),(1,1),(2,0),(3,0)):
 c=CPU();c.write(0x20005c72,section,1);c.write(0x20005c73,page,1);c.r[:2]=[2,1];sp=c.r[13]
 c.run(0x167d8,{0x167dc});assert c.r[13]==sp-16 and c.r[6]==0x20005c70
print('Menu event dispatch, timer bounds and stock fallback OK')

# Execute compiled renderer against stock widget API stubs; assert exact labels/values.
for selected in range(7):
 for timer in range(9):
  c=CPU();c.write(0x200089d6,codes[selected],2);c.write(0x20005ca0,0x20020000)
  labels={};values={};ranges={};positions={}
  c.stubs[0x322c0]=lambda c:c.r[1]
  c.stubs[0x2b35e]=lambda c:labels.update({c.r[0]:c.string(c.r[1])})
  c.stubs[0x20174]=lambda c:values.update({c.r[0]:c.string(c.r[1])})
  c.stubs[0x200c4]=lambda c:ranges.update({c.r[0]:(c.r[1],c.r[2])})
  c.stubs[0x20280]=lambda c:positions.update({c.r[0]:c.r[1]})
  c.stubs[0x28f68]=lambda c:0xabcdef
  c.stubs[0x20222]=lambda c:0
  c.write(0x20030000,int.from_bytes(b'DISP','little'));c.write(0x20030004,0)
  c.stubs[0x1bcdc]=lambda c:0x20030000
  c.stubs[0x16670]=lambda c:timer
  c.run(symbols['display_hook'],{0x17a70})
  assert labels[0x811]=='BUTTON COLOR' and labels[0x812]=='DISP' and labels[0x813]==''
  assert values[0x814]==names[selected]
  assert values[0x815]==('Always On','1Min','5Min','10Min','20Min','30Min','40Min','50Min','60Min')[timer]
  assert positions[0x814]==selected and positions[0x815]==timer
  assert ranges[0x814]==(0,6) and ranges[0x815]==(0,8)
print('Renderer: 63 colour/time combinations OK')
# Execute actual added mode-name hook and both displaced prologue trampolines.
for mode,stop in ((0,0x179bc),(1,0x17940),(2,0x1794e)):
 c=CPU();c.write(c.r[13]+0x98,mode,2);c.r[4]=0x814;shown=[]
 c.stubs[0x20174]=lambda c:shown.append((c.r[0],c.string(c.r[1])))
 c.run(0x1793a,{stop})
 assert shown==([(0x814,'MODE 3')] if mode==2 else [])
for sym,stop,stack_bytes in (('native_update',0x1f234,36),('native_input',0x1fc58,16)):
 c=CPU();c.r[0]=0x20010000;sp=c.r[13];c.run(symbols[sym],{stop})
 assert c.r[13]==sp-stack_bytes
 if sym=='native_input':assert c.r[4]==0x20010000
# Main template: original 35 records byte-identical; added IDs unique and in bounds.
template=symbols['home_template'];blob=itcm[template:template+51*28]
assert blob[:980]==stock[0x4c390+0x48c:0x4c390+0x48c+980]
ids=[]
for i in range(51):
 fn,txt,ident,x,y,w,h,flags,extra,param=struct.unpack_from('<II6HII',blob,i*28)
 ids.append(ident)
 if 35<=i<46:assert fn==0x2b213 and ident==0x850+i-35 and x+w<=800 and y+h<=480
for i,source in ((46,0x48c+25*28),(47,0x160c),(48,0x860+56)):
 record=bytearray(blob[i*28:(i+1)*28]);original=bytearray(stock[0x4c390+source:0x4c390+source+28])
 record[8:18]=original[8:18]
 assert record==original
 assert ids[i]==0x85b+i-46
for i,source in ((49,0x1660),(50,0x16d0)):
 record=bytearray(blob[i*28:(i+1)*28]);original=bytearray(stock[0x4c390+source:0x4c390+source+28])
 assert struct.unpack_from('<H',record,8)[0]==0x85e+i-49
 record[8:10]=original[8:10]
 assert record==original # exact Drum label/selector geometry and parameters
assert struct.unpack_from('<4H',blob,46*28+10)==(20,92,760,276)
# BACK is taken from the actual Drum template and shares its font, skin and string.
assert struct.unpack_from('<I',itcm,0x9c34)[0]==0x800015f0
assert struct.unpack_from('<I',itcm,0x44374)[0]==0x803209a8
assert struct.unpack_from('<I',itcm,0x44378)[0]==0x3a471
assert len(ids)==len(set(ids))
assert struct.unpack_from('<I',itcm,0x1fc4c)[0]==symbols['mode3_callback']
assert struct.unpack_from('<I',itcm,0x1fc50)[0]==template
assert itcm[0x1fc2e:0x1fc30]==bytes.fromhex('3321')
assert template>=symbols['mode3_state']+76
print('Mode 3 ARM hooks/trampolines, original template and 11 labels, native list, Drum BACK and native Drum COLOR selector OK')
assert len(out)==len(stock)+report['extra_bytes']
assert out[14]==sum(out[48:])%256
print('Container checks and inverse reconstruction passed; NOT tested on hardware.')

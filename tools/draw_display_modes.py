#!/usr/bin/env python3
"""Draw original vector illustrations of the Mini's display layouts (not screenshots)."""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'docs/images';OUT.mkdir(parents=True,exist_ok=True)

def rect(x,y,w,h,fill,stroke='none',r=2):
 return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{fill}" stroke="{stroke}" stroke-width="2"/>'
def text(x,y,value,size=24,fill='#f2f5f2',anchor='start',weight=500):
 return f'<text x="{x}" y="{y}" font-size="{size}" fill="{fill}" text-anchor="{anchor}" font-weight="{weight}">{value}</text>'
def screen(mode):
 s=rect(0,0,800,480,'#090b0b')
 for x,label in [(20,'CTRL/EXP'),(212,'DRUM'),(404,'GLOBAL'),(596,'EDIT')]:
  s+=rect(x,12,184,68,'#2f5058','#82cbd2')+text(x+92,55,label,25,anchor='middle',weight=650)
 s+=rect(20,94,310,49,'#090b0b','#647a75')+text(31,127,'OUT',23)+text(212,127,'IN',23)
 for x in (92,246):
  for y in (108,124):
   s+=rect(x,y,58,5,'#253331')+rect(x,y,35 if y==108 else 23,5,'#76cc99')
 s+='<path d="M194 103v31" stroke="#647a75" stroke-width="2"/>'
 if mode==2:
  s+=rect(346,94,266,49,'#111717','#3d4945')+text(479,126,'HOLD TO LOCK',20,'#9ba99f','middle')+text(780,126,'BPM:120',22,anchor='end')
  s+=rect(20,156,760,206,'#151b1a','#5b6862')
  s+=rect(20,156,61,206,'#26312d')+rect(719,156,61,206,'#26312d')
  s+=text(50,266,'‹',53,anchor='middle')+text(750,266,'›',53,anchor='middle')
  s+=text(400,252,'TIM CLEAN',62,anchor='middle',weight=650)+text(400,329,'P28-2',28,'#b9cac1','middle')
  for x,label,value,amount in [(20,'AMP Gain','40',91),(277,'DLY Mix','16',36),(534,'AMP Master','65',148)]:
   s+=rect(x,378,246,90,'#151b1a','#4b5f56')+text(x+123,407,label,22,anchor='middle')
   s+=rect(x+12,420,222,34,'#253330')+rect(x+12,420,amount,34,'#668e89')+text(x+123,445,value,23,anchor='middle')
 else:
  for x,letter,num,name,fill,ink in [(20,'A','P28-2','TIM CLEAN','#8bd7e1','#091c23'),(408,'B','P28-3','TIM BRIGHT','#293e5c','#f3f4f5')]:
   s+=rect(x,156,372,312,fill)
   s+=text(x+352,197,num,31,ink,'end',650)+text(x+352,304,letter,96,ink,'end',700)+text(x+20,439,name,35,ink,weight=600)
 return s
for mode in (2,3):
 (OUT/f'mode-{mode}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 800 480" role="img" aria-labelledby="title"><title id="title">Illustrated Ampero Mini Mode {mode}</title><g font-family="Arial, Helvetica, sans-serif">'+screen(mode)+'</g></svg>\n')
s='<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1680 590" role="img" aria-labelledby="title"><title id="title">Ampero Mini display comparison: Mode 2 on the left, Mode 3 on the right</title>'+rect(0,0,1680,590,'#101214')+'<g font-family="Arial, Helvetica, sans-serif">'
for x,mode in [(20,2),(860,3)]:
 s+=rect(x-8,12,816,496,'#060909','#424c4b',14)
 s+=f'<g transform="translate({x} 20)">'+screen(mode)+'</g>'
 s+=text(x+400,558,f'MODE {mode}',27,'#c5d2cc','middle',700)
s+='</g></svg>\n'
(OUT/'display-modes.svg').write_text(s)
print('Drew Mode 2, Mode 3 and a side-by-side README illustration.')
if __name__=='__main__':pass

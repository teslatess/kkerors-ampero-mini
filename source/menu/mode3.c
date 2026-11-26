/* MP-50 V2.2 Mode 3: two saved presets. All native addresses are pinned by build.py.
 * No audio algorithms, bootloader or direct flash routines are changed.
 * This file is also compiled on the host with mocked device services by mode3_test.c.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef __UINTPTR_TYPE__ uptr;
#define LABEL_FIRST 0x850
#define LABEL_COUNT 11
#define HOLD_TICKS 600u /* Same home-update counter convention as stock 0x1faf6. */
typedef struct {
 u32 window, active, initialized, chooser, candidate, touch, ticks, held;
 u32 fsHeld, lastPreset, lastA, lastB, wasMode3, dirty, lastX, lastY, listReady, fsPressed, candidateColor;
} State;
_Static_assert(sizeof(State)==76,"assembly state allocation must match");
extern State mode3_state;
#define S mode3_state
#ifdef HOST_TEST
extern u16 host_mode,host_pair[2];
extern u32 host_dirty;
#define MODE host_mode
#define PAIR host_pair
#define DIRTY host_dirty
u32 native_widget(u32,u32);void native_hide(u32);void native_show(u32);
void native_rect(u32,int,int,int,int);void native_text(u32,const char*);
void native_font(u32,uptr);void native_align(u32,u32);
void native_fg(u32,u32);void native_bg(u32,u32);void native_invalidate(u32);
u32 native_current(void);const char* native_name(u32);const char* native_number(u32);
void native_load(u32,u32,uptr);void native_setmode(u32);
void native_callback(u32*);void native_update(void);u32 native_input(const u8*);
void native_paintcolor(u32);void native_fill(int,int,int,int);
void native_outline(int,int,int,int,int);
u32 native_id(u32);u32 native_list_count(u32);u32 native_list_clicked(u32);
u32 native_list_add(u32,const char*,u32);void native_list_scroll(u32,u32);
void native_list_font(u32,u32,uptr);void native_list_fg(u32,u32,u32);void native_list_bg(u32,u32,u32);
void native_button_font(u32,uptr);void native_button_text(u32,const char*);
void native_selector_range(u32,u32,u32);void native_selector_value(u32,u32);
u32 native_selector_get(u32);void native_selector_text(u32,const char*);
void native_selector_font(u32,uptr);void native_selector_fg(u32,u32,u32);
const char* native_string(u32);
u32 native_list_draw(u32*);void native_list_setdraw(u32,uptr);
u32* native_lock(u32);void native_unlock(u32**);void native_gui_enter(void);void native_gui_leave(void);
void native_button_skin(u32,uptr);void native_button_fg(u32,u32,u32);u32 native_theme(u32);

#else
#define MODE (*(volatile u16*)0x20008938)
#define PAIR ((volatile u16*)0x200089d8)
#define DIRTY (*(volatile u32*)0x200061a0)
#define NATIVE(name,addr,ret,args) static ret (*const name) args=(ret (*) args)((addr)|1u)
NATIVE(native_widget,0x322c0,u32,(u32,u32));
NATIVE(native_hide,0x32848,void,(u32));NATIVE(native_show,0x331b6,void,(u32));
NATIVE(native_rect,0x33104,void,(u32,int,int,int,int));
NATIVE(native_text,0x2b35e,void,(u32,const char*));NATIVE(native_font,0x2b330,void,(u32,uptr));
NATIVE(native_align,0x2b3b2,void,(u32,u32));NATIVE(native_fg,0x2b3dc,void,(u32,u32));
NATIVE(native_bg,0x2b2f0,void,(u32,u32));NATIVE(native_invalidate,0x3298c,void,(u32));
NATIVE(native_current,0x13990,u32,(void));NATIVE(native_name,0x138f0,const char*,(u32));
NATIVE(native_number,0x1bcd0,const char*,(u32));NATIVE(native_load,0x22d68,void,(u32,u32,uptr));
NATIVE(native_setmode,0x28584,void,(u32));NATIVE(native_callback,0x46928,void,(u32*));
extern void native_update(void);extern u32 native_input(const u8*);
NATIVE(native_id,0x32356,u32,(u32));
NATIVE(native_selector_range,0x200c4,void,(u32,u32,u32));NATIVE(native_selector_value,0x20280,void,(u32,u32));
NATIVE(native_selector_get,0x20044,u32,(u32));NATIVE(native_selector_text,0x20174,void,(u32,const char*));
NATIVE(native_selector_font,0x2009c,void,(u32,uptr));NATIVE(native_selector_fg,0x20222,void,(u32,u32,u32));
NATIVE(native_string,0x1bcdc,const char*,(u32));
NATIVE(native_list_draw,0x259d4,u32,(u32*));NATIVE(native_list_setdraw,0x25e50,void,(u32,uptr));
NATIVE(native_lock,0xefcc,u32*,(u32));NATIVE(native_unlock,0xf084,void,(u32**));
NATIVE(native_gui_enter,0x10a9c,void,(void));NATIVE(native_gui_leave,0x11e34,void,(void));
NATIVE(native_list_count,0x258d8,u32,(u32));NATIVE(native_list_clicked,0x25906,u32,(u32));
NATIVE(native_list_add,0x25530,u32,(u32,const char*,u32));NATIVE(native_list_scroll,0x25ed8,void,(u32,u32));
NATIVE(native_list_font,0x25d38,void,(u32,u32,uptr));NATIVE(native_list_fg,0x25fac,void,(u32,u32,u32));
NATIVE(native_list_bg,0x25d02,void,(u32,u32,u32));NATIVE(native_button_font,0x46be,void,(u32,uptr));
NATIVE(native_button_text,0x4744,void,(u32,const char*));NATIVE(native_button_skin,0x46f8,void,(u32,uptr));
NATIVE(native_button_fg,0x47c8,void,(u32,u32,u32));NATIVE(native_theme,0x28f68,u32,(u32));
NATIVE(native_outline,0xff9c,void,(int,int,int,int,int));
NATIVE(native_paintcolor,0x115e2,void,(u32));NATIVE(native_fill,0xfe4c,void,(int,int,int,int));
#endif
static const u16 hidden[]={0x805,0x806,0x807,0x812,0x815,0x816,0x817,
 0x821,0x822,0x823,0x831,0x832,0x833,0x834,0x835,0x836,0x837,0x838,0x839,0x844,0x845,0x846};
static const u16 rects[LABEL_COUNT][4]={
 {20,156,374,324},{406,156,374,324}, {264,210,110,110},{650,210,110,110},
 {40,400,334,60},{426,400,334,60},{214,162,160,46},{600,162,160,46},
 {40,340,334,60},{426,340,334,60},{332,86,448,64}};
/* emWin colours are 0x00BBGGRR. */
#define BLACK 0u
#define BG 0x00141010u
#define CYAN 0x00c7ba35u
#define PURPLE 0x00d893b1u
#define INACTIVE 0x00443530u
#define WHITE 0x00ffffffu
#define DARK 0x00231c17u
#define COLOR_COUNT 7u
static const u32 palette[7]={0x005bb6d7u,0x006c68dcu,0x00da9b6fu,PURPLE,CYAN,0x0083b591u,0x007298dfu};
static const char*const patch_color_names[7]={"Gold","Red","Blue","Purple","Cyan","Green","Orange"};
static u32 widget(u32 i){return native_widget(S.window,LABEL_FIRST+i);}
static void persist(void){DIRTY=10;}
/* Each halfword carries a distinct tag; arbitrary stock/reserved values are rejected. */
u32 mode3_valid(u32 word,u32 slot){return slot<2 && (word&0xf800u)==(slot?0xb000u:0xa000u) && (word&255u)<198u;}
u32 mode3_pair(u32 slot){u32 v=PAIR[slot];return mode3_valid(v,slot)?v&255u:slot;}
static void ensure_pair(void){
 for(u32 i=0;i<2;i++)if(!mode3_valid(PAIR[i],i)){PAIR[i]=(u16)((i?0xb000u:0xa000u)|i);persist();}
}
u32 mode3_color(u32 slot){
 u32 word=PAIR[slot],encoded=(word>>8)&7u;
 return mode3_valid(word,slot)&&encoded?encoded-1u:(slot?3u:4u);
}
void mode3_assign_color(u32 slot,u32 preset,u32 color){
 if(slot>1 || preset>=198 || color>=COLOR_COUNT)return;
 PAIR[slot]=(u16)((slot?0xb000u:0xa000u)|((color+1u)<<8)|preset);persist();S.dirty=1;
}
void mode3_assign(u32 slot,u32 preset){
 if(slot>1 || preset>=198)return;
 mode3_assign_color(slot,preset,mode3_color(slot));
}
void mode3_select(u32 slot){
 if(slot>1 || MODE!=2 || S.chooser)return;
 u32 p=mode3_pair(slot);S.active=slot;S.dirty=1;
 if(native_current()!=p)native_load(p,1,0x200089e0u);
}
static void label(u32 i,const char*text,u32 fg,u32 bg,uptr font){
 u32 w=widget(i);native_bg(w,bg);native_fg(w,fg);native_font(w,font);
 native_align(w,0x0e); /* native H center (2) + V center (12) */native_text(w,text);native_show(w);
}
static void place(u32 i,int x,int y,int w,int h){native_rect(widget(i),x,y,w,h);}
static void layout(void){
 for(u32 i=0;i<LABEL_COUNT;i++)place(i,rects[i][0],rects[i][1],rects[i][2],rects[i][3]);
}
static void hide_stock(void){for(u32 i=0;i<sizeof(hidden)/sizeof(hidden[0]);i++)native_hide(native_widget(S.window,hidden[i]));}
/* The list owns scrolling, hit testing and drag cancellation, exactly as stock.
 * Row strings are copied by 0x25530; the name getter's scratch pointer is not retained.
 */
#define LIST_ID 0x85b
#define CANCEL_ID 0x85c
#define SAVE_ID 0x85d
#define COLOR_LABEL_ID 0x85e
#define COLOR_SELECTOR_ID 0x85f
#define CHOOSER_LAST COLOR_SELECTOR_ID
static void control_text(u32 id,const char*text,u32 fg,u32 bg,u32 align){
 u32 w=native_widget(S.window,id);native_bg(w,bg);native_fg(w,fg);native_font(w,0x803209a8u);
 native_align(w,align);native_text(w,text);native_show(w);
}
/* The original list keeps scrolling, sizing and hit testing. Only the two row
 * paint commands temporarily receive theme colors, never invalidating in paint.
 * This also highlights the candidate after a drag or a scroll clears native focus.
 */
u32 mode3_list_draw(u32*info){
 if(!S.chooser || (info[1]!=5 && info[1]!=18))return native_list_draw(info);
 u32 saved[8],selected=info[2]==S.candidate;
 u32 fg=selected?native_theme(24):WHITE,bg=selected?native_theme(17):BLACK;
 native_gui_enter();u32*obj=native_lock(info[0]);
 for(u32 i=0;i<8;i++){saved[i]=obj[16+i];obj[16+i]=i<5?fg:bg;}
 u32 result=native_list_draw(info);
 for(u32 i=0;i<8;i++)obj[16+i]=saved[i];
 native_unlock(&obj);native_gui_leave();return result;
}
static void chooser_footer(void){
 control_text(COLOR_LABEL_ID,"COLOR",native_theme(1),BLACK,0x0e);
 u32 w=native_widget(S.window,COLOR_SELECTOR_ID);
 native_selector_font(w,0x803209a8u);native_selector_range(w,0,COLOR_COUNT-1);
 native_selector_value(w,S.candidateColor);native_selector_text(w,patch_color_names[S.candidateColor]);
 native_selector_fg(w,0,native_theme(1));native_show(w);
}
static void change_color(u32 action){
 if(action==1 && S.candidateColor>0)S.candidateColor--;
 if(action==2 && S.candidateColor+1<COLOR_COUNT)S.candidateColor++;
 chooser_footer();
}
static u32 control(u32 id){return native_widget(S.window,id);}
static void preset_text(char*dst,u32 preset){
 u32 n=0;const char*t=native_number(preset);
 while(*t && n<12)dst[n++]=*t++;
 dst[n++]=' ';dst[n++]=' ';
 t=native_name(preset);while(*t && n<61)dst[n++]=*t++;dst[n]=0;
}
static void chooser_title(void){
 place(10,180,0,440,80);label(10,S.chooser==1?"PRESET A":"PRESET B",WHITE,BLACK,0x803209a8u);
}
static void chooser_list(void){
 u32 list=control(LIST_ID);
 if(!S.listReady){
  for(u32 state=0;state<3;state++)native_list_font(list,state,0x803209a8u);
  for(u32 state=0;state<3;state++){
   native_list_bg(list,state,BLACK);
  }
  /* Text has five native states; background has three (different APIs). */
  for(u32 state=0;state<5;state++)native_list_fg(list,state,WHITE);
  for(u32 p=native_list_count(list);p<198;p++){
   char text[64];preset_text(text,p);native_list_add(list,text,60);
  }
  native_list_setdraw(list,(uptr)mode3_list_draw);
  S.listReady=native_list_count(list)==198;
 }
 native_list_scroll(list,S.candidate);native_show(list);
 for(u32 id=CANCEL_ID;id<=SAVE_ID;id++){
  u32 w=control(id);native_button_font(w,0x803209a8u);
  native_button_fg(w,0,native_theme(1));
  native_button_text(w,id==CANCEL_ID?native_string(0x13):"SAVE");
  native_button_skin(w,0x3a471u);native_show(w);
 }
 chooser_title();chooser_footer();
}
/* ASCII glyph advance is byte 1 in each native GUI_CHARINFO record.
 * Pinned to the exact V2.2 60-pixel name font; no guessed character count.
 */
static u32 name_advance(u8 c,u32 compact){
#ifdef HOST_TEST
 extern u32 host_name_advance(u8,u32);
 return host_name_advance(c,compact);
#else
 if(c<32 || c>126)c='?';
 return *((const u8*)(compact?0x80161864u:0x801f4f24u)+(c-32u)*8u+1u);
#endif
}
static u32 split_name(char lines[2][64],const char*name,u32 compact){
 lines[0][0]=lines[1][0]=0;
 u32 line=0,n=0,width=0;
 for(u32 i=0;name[i] && i<63;i++){
  u8 c=(u8)name[i];u32 advance=name_advance(c,compact);
  if(width+advance>334u && n){
   lines[line][n]=0;
   if(line==1)return 0;
   line=1;n=width=0;
  }
  lines[line][n++]=(char)c;width+=advance;
 }
 lines[line][n]=0;
 /* A short name sits on the bottom baseline; a wrapped name grows upward. */
 if(!line){for(u32 i=0;i<=n;i++)lines[1][i]=lines[0][i];lines[0][0]=0;}
 return 1;
}
static uptr name_lines(char lines[2][64],const char*name){
 if(split_name(lines,name,0))return 0x801f5ff4u;
 split_name(lines,name,1);return 0x80162934u;
}
static void preset_label(u32 side,u32 preset,u32 fg,u32 bg){
 char lines[2][64];uptr font=name_lines(lines,native_name(preset));
 u32 height=font==0x80162934u?42u:60u,x=side?426u:40u;
 place(8+side,x,460-2*height,334,height);place(4+side,x,460-height,334,height);
 label(8+side,lines[0],fg,bg,font);native_align(widget(8+side),0);
 label(4+side,lines[1],fg,bg,font);native_align(widget(4+side),0);
}
static void render(void){
 if(!S.window || MODE!=2)return;
 hide_stock();layout();
 for(u32 i=8;i<LABEL_COUNT;i++)native_hide(widget(i));
 if(S.chooser){
  for(u32 i=0;i<LABEL_COUNT;i++)native_hide(widget(i));
  for(u32 id=0x801;id<=0x804;id++)native_hide(control(id));
  for(u32 id=0x83c;id<=0x843;id++)native_hide(control(id));
  chooser_list();
 }else{
  for(u32 id=LIST_ID;id<=CHOOSER_LAST;id++)native_hide(control(id));
  for(u32 id=0x801;id<=0x804;id++)native_show(control(id));
  for(u32 id=0x83c;id<=0x843;id++)native_show(control(id));
  u32 current=native_current();
  if(current==mode3_pair(0) && current!=mode3_pair(1))S.active=0;
  if(current==mode3_pair(1) && current!=mode3_pair(0))S.active=1;
  for(u32 side=0;side<2;side++){
   u32 p=mode3_pair(side),on=current==p && S.active==side;
   u32 bg=on?palette[mode3_color(side)]:INACTIVE,fg=on?DARK:WHITE;
   label(side,"",fg,bg,0x80162934u);
   label(2+side,side?"B":"A",fg,bg,0x801da0b0u);
   native_align(widget(2+side),0x0d); /* right-aligned letters, Quad Cortex reference */
   label(6+side,native_number(p),fg,bg,0x80162934u);
   native_align(widget(6+side),0x0d);
   preset_label(side,p,fg,bg);
  }
 }
 S.lastPreset=native_current();S.lastA=PAIR[0];S.lastB=PAIR[1];S.dirty=0;
 native_invalidate(S.window);
}
static void open_chooser(u32 side){S.chooser=side+1;S.candidate=mode3_pair(side);S.candidateColor=mode3_color(side);S.dirty=1;}
static void close_chooser(u32 save){
 if(save && S.candidate<198)mode3_assign_color(S.chooser-1,S.candidate,S.candidateColor);
 S.chooser=0;S.touch=S.ticks=S.held=S.fsHeld=S.fsPressed=0;S.dirty=1;
}
/* HOME update runs stock processing first, preserving meters and navigation. */
void mode3_update(void){
 native_update();
 if(MODE!=2){
  if(S.wasMode3){for(u32 i=0;i<LABEL_COUNT;i++)native_hide(widget(i));for(u32 id=LIST_ID;id<=CHOOSER_LAST;id++)native_hide(control(id));S.wasMode3=0;}
  S.touch=S.ticks=S.held=S.fsHeld=S.fsPressed=S.chooser=0;return;
 }
 if(!S.window)return;
 if(!S.wasMode3){ensure_pair();S.wasMode3=1;S.dirty=1;}
 if(S.initialized){S.initialized=0;if(native_current()!=mode3_pair(0) && native_current()!=mode3_pair(1))mode3_select(0);}
 if((S.touch==1 || S.touch==2) && !S.chooser && !S.held){
  if(++S.ticks>=HOLD_TICKS){S.held=1;open_chooser(S.touch-1);}
 }
 if(S.dirty || S.lastPreset!=native_current() || S.lastA!=PAIR[0] || S.lastB!=PAIR[1])render();
}
void mode3_callback(u32*message){
 /* Stock WM_PAINT draws frames directly, even for hidden stock children.
  * Own the Mode 3 parent canvas so no old frames survive in the tile gap or
  * beside the meters. Other messages still reach the native callback.
  */
 if(MODE==2 && message[0]==0xf){
  native_paintcolor(S.chooser?BLACK:BG);native_fill(0,0,799,479);
  if(S.chooser){
   native_paintcolor(native_theme(20));
   native_outline(20,370,266,470,3);
   native_outline(277,370,523,470,3);
   native_outline(532,370,780,470,3);
  }else{
   native_paintcolor(native_theme(20));
   native_outline(20,86,320,150,3); /* original OUT/IN frame only */
   native_fill(170,98,170,138); /* original OUT/IN divider */
  }
  return;
 }

 if(MODE==2 && message[0]==0x26){
  u32 id=native_id(message[2]);
  if(id>=LIST_ID && id<=CHOOSER_LAST){
   if(S.chooser && id==COLOR_SELECTOR_ID && message[3]==5){
    u32 color=native_selector_get(message[2]);
    if(color<COLOR_COUNT){S.candidateColor=color;chooser_footer();}
   }else if(S.chooser && message[3]==2){
    if(id==LIST_ID){u32 p=native_list_clicked(message[2]);if(p<198){S.candidate=p;native_invalidate(control(LIST_ID));}}
    else if(id==CANCEL_ID || id==SAVE_ID)close_chooser(id==SAVE_ID);
   }
   return;
  }
 }
 native_callback(message);
 if(message[0]==0x1d){
  S.window=message[1];S.active=0;S.touch=S.ticks=S.held=S.fsHeld=S.fsPressed=S.chooser=0;
  S.wasMode3=0;S.dirty=1;S.initialized=1;S.listReady=0;
  for(u32 i=0;i<LABEL_COUNT;i++)native_hide(widget(i));
  for(u32 id=LIST_ID;id<=CHOOSER_LAST;id++)native_hide(control(id));
  if(MODE==2){ensure_pair();S.wasMode3=1;render();}
 }
}
/* Raw touch coordinates are transformed exactly as in the stock home input.
 * Touch press is boolean; released coordinates may be zero, so retain the target.
 */
u32 mode3_input(const u8*event){
 if(MODE!=2)return native_input(event);
 u32 kind=event[7]>>4,key=event[7]&15,state=event[6];
 if(kind==4 && key<2){
  u32 bit=1u<<key;
  if(state==1){S.fsHeld&=~bit;S.fsPressed|=bit;return 0;}
  /* Native key states: 1=down, 2=long action, 4=release.
   * Long actions only latch suppression; neither the action nor its release
   * may load a preset. Native kind 5 owns combined-switch detection, so a
   * stale pressed bit cannot misclassify the next single press as a combo.
   */
  if(state==2){S.fsHeld|=bit;return 0;}
  if(state==4){if(!(S.fsHeld&bit))mode3_select(key);S.fsHeld&=~bit;S.fsPressed&=~bit;return 0;}
  return 0;
 }
 /* Preserve native combined-switch actions (including tuner), while swallowing
  * the following individual releases so they cannot switch A/B. */
 if(kind==5){S.fsHeld=3;return native_input(event);}
 if(kind!=2)return native_input(event);
 u32 x=(u32)event[0]|((u32)event[1]<<8);
 u32 rawY=(u32)event[2]|((u32)event[3]<<8);
 u32 y=rawY<=480?480-rawY:481;
 if(S.touch==4){if(!state)S.touch=0;return native_input(event);}
 if(state){S.lastX=x;S.lastY=y;}
 else if(S.touch && x==0 && rawY==0){x=S.lastX;y=S.lastY;}
 if(S.chooser){
  /* Swallow only the gesture which opened the chooser; new gestures go to GUI. */
  if(S.held && (S.touch==1 || S.touch==2)){
   if(!state)S.touch=S.ticks=S.held=0;
   return 0;
  }
  /* The native selector paints exactly as on Drum/BUTTON COLOR. Handle its
   * two tap halves here; list gestures remain delegated for their whole life.
   */
  if(S.touch==5 || S.touch==6){
   if(!state){u32 action=S.touch-4;S.touch=0;
    if(x>=33 && x<253 && y>=415 && y<465 && ((x>=143)==(action==2)))change_color(action);
   }
   return 0;
  }
  if(state && x>=33 && x<253 && y>=415 && y<465){S.touch=x<143?5:6;return 0;}
  if(state)S.touch=4;
  return native_input(event);
 }
 if(S.touch){
  if(state)return 0;
  u32 side=S.touch-1,held=S.held;S.touch=S.ticks=S.held=0;
  if(!held && x>=20 && x<780 && y>=156 && y<480 && ((x>=406)==side))mode3_select(side);
  return 0;
 }
 if(x>=20 && x<780 && y>=156 && y<480){
  if(state && (x<394 || x>=406)){S.touch=x>=406?2:1;S.ticks=S.held=0;}
  return 0;
 }
 if(state)S.touch=4;
 return native_input(event);
}
/* First Display page: extend only DISPLAY MODE; language and theme remain native. */
u32 mode3_menu(u32 column,u32 action){
#ifdef HOST_TEST
 extern u8 host_section,host_page;
 u32 section=host_section,page=host_page;
#else
 u32 section=*(volatile u8*)0x20005c72,page=*(volatile u8*)0x20005c73;
#endif
 if(section!=3 || page!=0 || column!=0)return 0;
 u32 mode=MODE;
 if(action==1 && mode>0)mode--;
 if(action==2 && mode<2)mode++;
 if(mode!=MODE){native_setmode(mode);persist();}
 return 1;
}

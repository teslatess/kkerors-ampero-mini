/* Host execution of the SAME mode3.c logic; stock functions mocked, no ARM/device emulation. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define HOST_TEST 1
#include "mode3.c"
State mode3_state;
u16 host_mode,host_pair[2];u32 host_dirty;u8 host_section,host_page;
static u32 outlineCount,paintColor;
static u32 current,loads,inputs,updates,callbackCount,paintCount,visibility[0x900],nativeCalls;
static u32 rowCount,clickedRow,scrollRow,buttonStyles;
static uptr fonts[0x900];
static u32 listFg[5],listBg[3],alignments[0x900];
static char rowTexts[198][64];
static char texts[0x900][80];static int positions[0x900][4];
u32 native_widget(u32 win,u32 id){assert(win==123);assert(id<0x900);return id;}
void native_hide(u32 id){assert(id<0x900);visibility[id]=0;}
void native_show(u32 id){assert(id<0x900);visibility[id]=1;}
void native_rect(u32 id,int x,int y,int w,int h){assert(id<0x900);assert(x>=0&&y>=0&&x+w<=800&&y+h<=480);positions[id][0]=x;positions[id][1]=y;positions[id][2]=w;positions[id][3]=h;}
void native_text(u32 id,const char*s){assert(id<0x900);assert(strlen(s)<80);strcpy(texts[id],s);}
void native_font(u32 id,uptr f){assert(id>=0x850&&id<=COLOR_LABEL_ID);assert(f==0x80162934u||f==0x801f5ff4u||f==0x801da0b0u||f==0x803209a8u);fonts[id]=f;}
void native_align(u32 id,u32 a){assert(id>=0x850&&id<=COLOR_LABEL_ID);assert(a==0||a==0x0c||a==0x0e||a==0x0d);alignments[id]=a;}
void native_fg(u32 id,u32 c){(void)id;assert(c<0x1000000);}
void native_bg(u32 id,u32 c){(void)id;assert(c<0x1000000);}
void native_invalidate(u32 w){assert(w==123||w==LIST_ID);nativeCalls++;}
u32 native_current(void){return current;}
const char* native_name(u32 p){static char buf[30];assert(p<198);snprintf(buf,sizeof(buf),"Preset %u",p);return buf;}
const char* native_number(u32 p){static char buf[10];assert(p<198);snprintf(buf,sizeof(buf),"%u",p);return buf;}
void native_load(u32 p,u32 flag,uptr dst){assert(p<198&&flag==1&&dst==0x200089e0u);current=p;loads++;}
void native_setmode(u32 m){assert(m<3);host_mode=(u16)m;}
void native_callback(u32*m){assert(m);callbackCount++;}
void native_update(void){updates++;}
u32 native_input(const u8*e){assert(e);inputs++;return 77;}
void native_paintcolor(u32 c){assert(c==BG||c==BLACK||c==20||c==WHITE||c==DARK||c==palette[0]||c==palette[1]||c==palette[2]||c==palette[3]||c==palette[4]||c==palette[5]||c==palette[6]);paintColor=c;}
void native_fill(int a,int b,int c,int d){
 assert((paintColor==BG||paintColor==BLACK)?(a==0&&b==0&&c==799&&d==479):(paintColor==20&&a==170&&b==98&&c==170&&d==138));paintCount++;
}
void native_outline(int a,int b,int c,int d,int radius){
 assert(paintColor==20&&radius==3&&((a==20&&b==86&&c==320&&d==150)||(b==370&&d==470&&((a==20&&c==266)||(a==277&&c==523)||(a==532&&c==780)))));outlineCount++;
}

u32 native_id(u32 w){return w;}
u32 native_list_count(u32 w){assert(w==LIST_ID);return rowCount;}
u32 native_list_clicked(u32 w){assert(w==LIST_ID);return clickedRow;}
u32 native_list_add(u32 w,const char*t,u32 height){assert(w==LIST_ID&&height==60&&rowCount<198);assert(strlen(t)<64);strcpy(rowTexts[rowCount++],t);return 0;}
void native_list_scroll(u32 w,u32 row){assert(w==LIST_ID&&row<198);scrollRow=row;}
void native_list_font(u32 w,u32 state,uptr font){assert(w==LIST_ID&&state<3&&font==0x803209a8u);}
void native_list_fg(u32 w,u32 state,u32 color){assert(w==LIST_ID&&state<5&&color<0x1000000);listFg[state]=color;}
void native_list_bg(u32 w,u32 state,u32 color){assert(w==LIST_ID&&state<3&&color<0x1000000);listBg[state]=color;}
void native_button_font(u32 w,uptr font){assert((w==CANCEL_ID||w==SAVE_ID)&&font==0x803209a8u);}
void native_button_text(u32 w,const char*t){assert(!strcmp(t,w==CANCEL_ID?"< BACK":"SAVE"));}
void native_button_skin(u32 w,uptr skin){assert((w==CANCEL_ID||w==SAVE_ID)&&skin==0x3a471u);buttonStyles++;}
void native_button_fg(u32 w,u32 state,u32 color){assert((w==CANCEL_ID||w==SAVE_ID)&&state<3&&color<0x1000000);}
static u32 selectorValue;static char selectorText[16];
void native_selector_range(u32 w,u32 min,u32 max){assert(w==COLOR_SELECTOR_ID&&min==0&&max==6);}
void native_selector_value(u32 w,u32 value){assert(w==COLOR_SELECTOR_ID&&value<7);selectorValue=value;}
u32 native_selector_get(u32 w){assert(w==COLOR_SELECTOR_ID);return selectorValue;}
void native_selector_text(u32 w,const char*t){assert(w==COLOR_SELECTOR_ID&&strlen(t)<sizeof(selectorText));strcpy(selectorText,t);}
void native_selector_font(u32 w,uptr font){assert(w==COLOR_SELECTOR_ID&&font==0x803209a8u);}
void native_selector_fg(u32 w,u32 state,u32 color){assert(w==COLOR_SELECTOR_ID&&state==0&&color==1);}
const char* native_string(u32 index){assert(index==0x13);return "< BACK";}
static u32 listObject[40],guiDepth;
u32* native_lock(u32 handle){assert(handle==LIST_ID&&guiDepth);return listObject;}
void native_unlock(u32**ptr){assert(*ptr==listObject&&guiDepth);}
void native_gui_enter(void){guiDepth++;}
void native_gui_leave(void){assert(guiDepth);guiDepth--;}
void native_list_setdraw(u32 handle,uptr draw){assert(handle==LIST_ID&&draw==(uptr)mode3_list_draw);}
u32 native_list_draw(u32*info){
 if(S.chooser&&(info[1]==5||info[1]==18)){
  assert(guiDepth);u32 selected=info[2]==S.candidate;
  for(u32 i=0;i<8;i++)assert(listObject[16+i]==(i<5?(selected?24:WHITE):(selected?17:BLACK)));
 }
 return 77;
}
u32 native_theme(u32 index){return index;}
u32 host_name_advance(u8 c,u32 compact){
 static const u8 smallWidths[95]={8,11,13,25,20,32,27,7,13,13,16,19,9,14,9,19,22,15,20,20,22,19,21,18,22,21,9,9,18,20,18,18,26,23,22,21,24,20,19,23,24,10,13,22,19,32,25,24,21,24,23,20,21,24,22,34,22,21,20,12,19,12,16,19,12,20,21,18,21,20,14,20,21,10,10,19,11,31,21,21,21,21,15,18,14,21,20,30,20,20,18,12,9,12,18};
 static const u8 widths[95]={11,15,19,36,29,45,38,11,18,18,23,27,12,21,12,28,31,21,29,28,32,28,29,26,31,29,13,13,26,29,26,26,38,33,32,29,34,29,28,32,34,15,18,32,27,46,36,34,31,34,32,29,30,34,32,49,32,30,29,17,28,17,23,28,18,28,30,25,30,28,20,29,30,14,14,28,16,44,30,29,30,30,21,26,21,30,28,42,28,28,26,17,13,17,26};
 if(c<32||c>126)c='?';
 return compact?smallWidths[c-32]:widths[c-32];
}
static void notify(u32 id,u32 code){u32 msg[4]={0x26,123,id,code};mode3_callback(msg);}
static void reset(void){memset(&S,0,sizeof(S));host_mode=2;host_pair[0]=0xa000;host_pair[1]=0xb001;host_dirty=0;current=0;loads=inputs=updates=rowCount=buttonStyles=0;S.window=123;memset(visibility,0,sizeof(visibility));}
static void fs(u32 key,u32 state){u8 e[8]={0};e[7]=(u8)(0x40|key);e[6]=(u8)state;mode3_input(e);}
static void touch(u32 x,u32 y,u32 down){u8 e[8]={0};u32 raw=480-y;e[0]=x;e[1]=x>>8;e[2]=raw;e[3]=raw>>8;e[7]=0x20;e[6]=down;mode3_input(e);}
static void click(u32 x,u32 y){touch(x,y,1);touch(x,y,0);}
int main(void){
 /* Character wrapping preserves the name exactly and keeps each line in bounds. */
 const char* samples[]={"", "Spring", "Captain 50 Clean", "Compulsive Drive", "Legendary 87", "iiiiiiiiiiiiiiii", "WWWWWWWWWWWWWWWW"};
 for(u32 sample=0;sample<sizeof(samples)/sizeof(samples[0]);sample++){
  char lines[2][64]={{0},{0}},joined[128];uptr font=name_lines(lines,samples[sample]);
  strcpy(joined,lines[0]);strcat(joined,lines[1]);assert(!strcmp(joined,samples[sample]));
  for(u32 line=0;line<2;line++){u32 width=0;for(u32 i=0;lines[line][i];i++)width+=host_name_advance((u8)lines[line][i],font==0x80162934u);assert(width<=334);}
 }
 for(u32 c=32;c<=126;c++){
  char name[17],lines[2][64],joined[128];memset(name,c,16);name[16]=0;
  uptr font=name_lines(lines,name);strcpy(joined,lines[0]);strcat(joined,lines[1]);assert(!strcmp(joined,name));
  for(u32 line=0;line<2;line++){u32 width=0;for(u32 i=0;lines[line][i];i++)width+=host_name_advance((u8)lines[line][i],font==0x80162934u);assert(width<=334);}
 }
 /* Exhaustive tag validation: all 65536 halfwords, both slots. */
 for(u32 slot=0;slot<2;slot++)for(u32 w=0;w<65536;w++)assert(mode3_valid(w,slot)==((w&0xf800)==(slot?0xb000:0xa000)&&(w&255)<198));
 reset();for(u32 a=0;a<198;a++)for(u32 b=0;b<198;b++){
  mode3_assign(0,a);mode3_assign(1,b);assert(mode3_pair(0)==a&&mode3_pair(1)==b);
  current=a;S.active=0;mode3_select(1);assert(current==b&&S.active==1);mode3_select(0);assert(current==a&&S.active==0);
 }
 u16 a=PAIR[0],b=PAIR[1];mode3_assign(2,0);mode3_assign(0,198);assert(PAIR[0]==a&&PAIR[1]==b);
 reset();mode3_assign(0,197);mode3_assign(1,99);memset(&S,0,sizeof(S));assert(mode3_pair(0)==197&&mode3_pair(1)==99);assert(host_dirty==10);
 /* Native key states verified against stock dispatch: down=1, long=2, release=4. */
 reset();for(int i=0;i<100;i++){
  u32 key=i%2;fs(key,1);u32 before=loads;fs(key,4);assert(current==key);
  before=loads;fs(key,1);fs(key,4);assert(loads==before);
 }
 reset();fs(1,1);assert(!loads);fs(1,4);assert(current==1&&loads==1);fs(0,1);fs(0,4);assert(current==0&&loads==2);
 /* Repeated holds do nothing; both switches remain usable immediately after. */
 for(u32 key=0;key<2;key++)for(u32 repeats=1;repeats<=100;repeats++){
  reset();mode3_assign(0,50);mode3_assign(1,197);fs(key,1);
  for(u32 i=0;i<repeats;i++)fs(key,2);
  fs(key,4);assert(current==0&&!loads&&!inputs&&!S.fsHeld&&!S.fsPressed);
  fs(key,1);fs(key,4);assert(current==mode3_pair(key)&&loads==1);
  fs(1-key,1);fs(1-key,4);assert(current==mode3_pair(1-key)&&loads==2);
 }
 /* An interrupted stream must not poison the other switch or the next press. */
 reset();fs(1,1);fs(1,2);fs(0,1);fs(0,4);assert(!S.fsPressed||S.fsPressed==2);fs(1,1);fs(1,4);assert(current==1&&loads==1);
 reset();fs(0,1);fs(1,1);u8 combo[8]={0};combo[7]=0x50;combo[6]=1;
 assert(mode3_input(combo)==77);combo[6]=4;assert(mode3_input(combo)==77);
 fs(0,4);fs(1,4);assert(!loads&&inputs==2);
 for(u32 state=1;state<=4;state++){
  reset();combo[6]=state;assert(mode3_input(combo)==77);fs(0,2);fs(1,2);fs(0,4);fs(1,4);assert(!loads&&inputs==1);
  fs(1,1);fs(1,4);assert(current==1&&loads==1);
 }
 reset();fs(1,4);assert(current==1&&loads==1);fs(0,4);assert(current==0&&loads==2);
 for(u32 mode=0;mode<2;mode++){reset();host_mode=mode;fs(0,1);fs(0,4);click(600,300);assert(!loads&&inputs==4);}
 /* Clicks select fixed slots; drag outside cancels; zeroed release retains target. */
 reset();click(600,300);assert(current==1&&loads==1);click(600,300);assert(loads==1);click(100,300);assert(current==0&&loads==2);
 reset();touch(600,300,1);touch(10,120,0);assert(!loads);touch(600,300,1);u8 release[8]={0};release[7]=0x20;mode3_input(release);assert(current==1);
 /* Long touch opens chooser once. Release doesn't accidentally choose a preset. */
 reset();touch(600,300,1);for(u32 i=0;i<HOLD_TICKS-1;i++)mode3_update();assert(!S.chooser);mode3_update();assert(S.chooser==2&&S.candidate==1);touch(600,300,0);assert(!loads&&S.touch==0);
 assert(rowCount==198&&scrollRow==1&&buttonStyles==2);
 for(u32 i=0;i<3;i++)assert(listBg[i]==BLACK);
 for(u32 i=0;i<5;i++)assert(listFg[i]==WHITE);
 assert(alignments[0x85a]==0x0e);
 assert(positions[0x85a][0]+positions[0x85a][2]/2==400);
 assert(positions[0x85a][1]+positions[0x85a][3]/2==40);
 for(u32 i=0;i<198;i++){char expected[64];snprintf(expected,sizeof(expected),"%u  Preset %u",i,i);assert(!strcmp(rowTexts[i],expected));}
 assert(!visibility[0x801]&&!visibility[0x83c]&&visibility[LIST_ID]);
 u32 countBefore=inputs;touch(400,300,1);touch(400,200,1);touch(400,200,0);assert(inputs==countBefore+3); /* native drag stream */
 clickedRow=197;notify(LIST_ID,2);assert(S.candidate==197&&!loads&&mode3_pair(1)==1);
 notify(CANCEL_ID,2);mode3_update();assert(!S.chooser&&mode3_pair(1)==1&&visibility[0x801]&&!visibility[LIST_ID]);
 touch(600,300,1);for(u32 i=0;i<HOLD_TICKS;i++)mode3_update();touch(600,300,0);
 assert(rowCount==198); /* no duplicate rows when reopened */
 clickedRow=99;notify(LIST_ID,2);notify(SAVE_ID,2);mode3_update();assert(!S.chooser&&mode3_pair(1)==99&&host_dirty==10);click(600,300);assert(current==99);
 /* All selectable rows; invalid/cancelled scroll releases are ignored. */
 reset();open_chooser(0);mode3_update();
 for(u32 i=0;i<198;i++){clickedRow=i;notify(LIST_ID,2);assert(S.candidate==i);}
 clickedRow=0xffffffffu;notify(LIST_ID,2);assert(S.candidate==197);notify(LIST_ID,4);assert(S.candidate==197);
 fs(0,1);fs(0,4);fs(1,1);fs(1,2);fs(1,4);assert(!loads&&!inputs);
 notify(SAVE_ID,2);assert(mode3_pair(0)==197);
 /* Color/preset combinations, legacy defaults and transactional BACK/SAVE. */
 reset();assert(mode3_color(0)==4&&mode3_color(1)==3);
 for(u32 slot=0;slot<2;slot++)for(u32 color=0;color<COLOR_COUNT;color++)for(u32 preset=0;preset<198;preset++){
  mode3_assign_color(slot,preset,color);assert(mode3_pair(slot)==preset&&mode3_color(slot)==color);
  mode3_assign(slot,197-preset);assert(mode3_pair(slot)==197-preset&&mode3_color(slot)==color);
 }
 reset();open_chooser(0);mode3_update();assert(!strcmp(texts[0x85a],"PRESET A"));
 for(u32 i=0;i<COLOR_COUNT;i++){selectorValue=i;notify(COLOR_SELECTOR_ID,5);assert(S.candidateColor==i&&mode3_color(0)==4&&!strcmp(selectorText,patch_color_names[i]));}
 selectorValue=99;notify(COLOR_SELECTOR_ID,5);assert(S.candidateColor==6);
 click(200,440);assert(S.candidateColor==6);for(u32 i=0;i<8;i++)click(60,440);assert(S.candidateColor==0);
 click(200,440);assert(S.candidateColor==1);touch(200,440,1);touch(400,300,0);assert(S.candidateColor==1);
 u32 delegated=inputs;touch(400,300,1);touch(200,440,1);touch(200,440,0);assert(inputs==delegated+3&&S.candidateColor==1);
 touch(60,440,1);u8 zeroRelease[8]={0};zeroRelease[7]=0x20;mode3_input(zeroRelease);assert(S.candidateColor==0);
 notify(CANCEL_ID,2);assert(mode3_color(0)==4);
 open_chooser(1);mode3_update();clickedRow=197;notify(LIST_ID,2);selectorValue=0;notify(COLOR_SELECTOR_ID,5);notify(SAVE_ID,2);
 assert(mode3_pair(1)==197&&mode3_color(1)==0);memset(&S,0,sizeof(S));S.window=123;assert(mode3_color(1)==0);
 /* Draw candidate with theme colors for every possible row; restore object fields. */
 open_chooser(1);
 for(u32 i=0;i<40;i++)listObject[i]=0x123456;
 for(u32 row=0;row<198;row++)for(u32 cmd=5;cmd<=18;cmd+=13){
  u32 info[8]={LIST_ID,cmd,row,0,0,0,0,0};assert(mode3_list_draw(info)==77&&!guiDepth);
  for(u32 i=0;i<40;i++)assert(listObject[i]==0x123456);
 }
 /* Keep header presses intact even if finger moves into the body. */
 reset();touch(500,30,1);touch(500,300,1);touch(500,300,0);assert(inputs==3&&!loads&&!S.touch);
 /* Init, layout, names, hidden stock elements, preserved header and meters. */
 reset();for(u32 i=0;i<0x900;i++)visibility[i]=1;u32 msg[4]={0x1d,123,0,0};mode3_callback(msg);assert(callbackCount&&S.wasMode3);
 for(u32 i=0;i<sizeof(hidden)/sizeof(hidden[0]);i++)assert(!visibility[hidden[i]]);
 for(u32 i=0x801;i<=0x804;i++)assert(visibility[i]);for(u32 i=0x83c;i<=0x843;i++)assert(visibility[i]);
 assert(!strcmp(texts[0x852],"A")&&!strcmp(texts[0x853],"B"));assert(!strcmp(texts[0x854],"Preset 0")&&!strcmp(texts[0x855],"Preset 1"));
 assert(alignments[0x854]==0&&alignments[0x858]==0);
 assert(fonts[0x852]==0x801da0b0u&&fonts[0x853]==0x801da0b0u);
 assert(positions[0x852][3]>=100&&positions[0x853][3]>=100);
 assert(positions[0x854][1]+positions[0x854][3]==460);
 assert(positions[0x856][1]+positions[0x856][3]<=positions[0x852][1]);
 msg[0]=0xf;u32 callbacksBefore=callbackCount,paintsBefore=paintCount,outlinesBefore=outlineCount;
 mode3_callback(msg);assert(callbackCount==callbacksBefore&&paintCount==paintsBefore+2&&outlineCount==outlinesBefore+1);
 S.chooser=1;mode3_callback(msg);assert(callbackCount==callbacksBefore&&paintCount==paintsBefore+3&&outlineCount==outlinesBefore+4);S.chooser=0;
 for(u32 mode=0;mode<2;mode++){host_mode=mode;mode3_callback(msg);assert(callbackCount==++callbacksBefore&&paintCount==paintsBefore+3);}host_mode=2;
 /* All standard 5-character factory/user numbers fit the 42px native font. */
 for(u32 side=0;side<2;side++)for(u32 bank=1;bank<=33;bank++)for(u32 slot=1;slot<=3;slot++){
  char number[16];snprintf(number,sizeof(number),"P-%02u-%u",bank,slot);u32 width=0;
  for(u32 i=0;number[i];i++)width+=host_name_advance((u8)number[i],1);
  assert(width<=(u32)positions[0x856+side][2]&&positions[0x856+side][3]>=42);
  assert(positions[0x856+side][0]+positions[0x856+side][2]==(side?760:374));
 }
 reset();host_pair[0]=0;host_pair[1]=65535;mode3_update();assert(PAIR[0]==0xa000&&PAIR[1]==0xb001);
 reset();mode3_update();host_mode=0;mode3_update();for(u32 i=0;i<LABEL_COUNT;i++)assert(!visibility[0x850+i]);assert(updates==2);
 /* Display selection boundaries; theme/language/other sections bypass this handler. */
 for(u32 m=0;m<3;m++)for(u32 action=0;action<3;action++){
  reset();host_mode=m;host_section=3;host_page=0;assert(mode3_menu(0,action)==1);
  assert(host_mode==(action==1?(m?m-1:m):action==2?(m<2?m+1:m):m));
 }
 reset();host_section=3;host_page=1;assert(!mode3_menu(0,1));host_page=0;assert(!mode3_menu(1,1));host_section=2;assert(!mode3_menu(0,1));
 puts("Mode 3 host checks passed: 131072 tag cases, 39204 A/B pairs, direct A/B footswitches, short releases, inert long actions and recovery, native combined-switch dispatch, native list 198 rows, 2772 color/preset cases, themed candidate rows, native selector, tap boundaries and drag ownership, BACK/SAVE transactions, persistence/layout/menu. Native services mocked; hardware untested.");
}

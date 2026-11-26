.balign 4
.global native_update
.thumb_func
native_update:
 push.w {r4-r11, lr}
 jump 0x1f234
.ltorg
.global native_input
.thumb_func
native_input:
 push {r4-r6, lr}
 mov r4,r0
 jump 0x1fc58
.ltorg
/* First page selector text is extended without altering language/theme widgets. */
.global mode_name_hook
.thumb_func
mode_name_hook:
 ldrh.w r0,[sp,#0x98]
 cmp r0,#2
 beq 1f
 cmp r0,#0
 beq 2f
 jump 0x17940
2:
 jump 0x179bc
1:
 ldr r1,=mode3_name
 mov r0,r4
 call 0x20174
 jump 0x1794e
.ltorg
mode3_name: .asciz "MODE 3"
.balign 4
/* Mutable state lives in the appended ITCM segment, copied to RAM on boot. */
.global mode3_state
mode3_state: .space 76
/* build.py fills this from the user's exact original, never committed vendor bytes. */
.global home_template
home_template: .space 980
.macro text_record ident,x,y,w,h
 .word 0x2b213, empty
 .hword \ident,\x,\y,\w,\h,0
 .word 0,0
.endm
 text_record 0x850,20,156,374,324
 text_record 0x851,406,156,374,324
 text_record 0x852,264,210,110,110
 text_record 0x853,650,210,110,110
 text_record 0x854,40,400,334,60
 text_record 0x855,426,400,334,60
 text_record 0x856,214,162,160,46
 text_record 0x857,600,162,160,46
 text_record 0x858,40,340,334,60
 text_record 0x859,426,340,334,60
 text_record 0x85a,332,86,448,64

/* Filled with native list, Drum BACK and CTRL/EXP ASSIGN records by build.py. */
.global chooser_controls
chooser_controls: .space 84

/* Exact native Drum label/selector records, populated by build.py. */
.global footer_controls
footer_controls: .space 56

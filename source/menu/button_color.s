.syntax unified
.cpu cortex-m7
.thumb
.section .text
.org 0x4c328
.macro call address
 ldr r12, =\address+1
 blx r12
.endm
.macro jump address
 ldr r12, =\address+1
 bx r12
.endm

/* Stored in the stock reserved halfword: global + 0x1e6 + 0xf0.
 * BC00 Cyan / BC01 legacy White->Cyan / BC02 Purple; new colors BC03..BC07. */
.global choice
.thumb_func
choice:
 ldr r1, =0x200089d6
 ldrh r0, [r1]
 movw r2, #0xbc00
 subs r0, r0, r2
 cmp r0, #7
 bhi 1f
 adr r1, stored_choices
 ldrb r0,[r1,r0]
 bx lr
1:
 movs r0,#4
 bx lr
.balign 4
stored_choices: .byte 4,4,3,0,1,2,5,6

.ltorg

.global validate_reserved
.thumb_func
validate_reserved:
 ldrh r0, [r4, #0xf0]
 movw r1, #0xbc00
 sub r0, r0, r1
 cmp r0, #7
 it hi
 strhhi r6, [r4, #0xf0]
 /* Validate independently tagged A/B assignments in the two remaining words. */
 ldrh r0,[r4,#0xf2]
 movw r1,#0xa000
 sub r0,r0,r1
 cmp r0,#0x800
 bhs 1f
 and r0,r0,#255
 cmp r0,#197
 bls 2f
1:
 strh r6,[r4,#0xf2]
2:
 ldrh r0,[r4,#0xf4]
 movw r1,#0xb000
 sub r0,r0,r1
 cmp r0,#0x800
 bhs 3f
 and r0,r0,#255
 cmp r0,#197
 bls 4f
3:
 strh r6,[r4,#0xf4]
4:
 jump 0x16dd0
.ltorg

.global restore_reserved
.thumb_func
restore_reserved:
 push {r4, lr}
 mov r4, r0
 call 0x16b44
 ldrh r1, [r4, #0xf0]
 ldr r2, =0x200089d6
 strh r1, [r2]
 ldrh r1,[r4,#0xf2]
 strh r1,[r2,#2]
 ldrh r1,[r4,#0xf4]
 strh r1,[r2,#4]
 pop {r4, pc}
.ltorg

.global led_hook
.thumb_func
led_hook:
 cmp r0, #5
 beq 1f
 cmp r0, #40
 bne 4f
1:
 ldr r3, =0x200089d6
 ldrh r3, [r3]
 movw r12, #0xbc00
 sub r3,r3,r12
 cmp r3,#7
 bhi 4f
 cmp r0,#5
 bne 2f
 adr r12,led_masks_left
 b 3f
2:
 adr r12,led_masks_right
3:
 ldrb r0,[r12,r3]
4:
 /* Displaced stock prologue, followed by untouched handler body. */
 push.w {r4-r9}
 jump 0x28208
.ltorg

.balign 4
led_masks_left: .byte 5,5,6,3,2,4,1,3
led_masks_right: .byte 40,40,48,24,16,32,8,24

/* Intercept only Display page 2. All other stock events use the stock body. */
.global event_hook
.thumb_func
event_hook:
 push {r0-r4,lr}
 bl mode3_menu
 cmp r0,#0
 beq 9f
 pop {r0-r4,pc}
9:
 pop {r0-r4,lr}
 ldr r2, =0x20005c70
 ldrb r3, [r2, #2]
 cmp r3, #3
 bne stock_event
 ldrb r3, [r2, #3]
 cmp r3, #0
 beq stock_event
 b menu_event
stock_event:
 push {r4-r6, lr}
 ldr r6, =0x20005c70
 jump 0x167dc
menu_event:
 push {r4-r6, lr}
 mov r4, r1
 cmp r0, #0
 beq color_event
 cmp r0, #1
 beq time_event
 b event_done
color_event:
 bl choice
 mov r5, r0
 cmp r4, #1
 beq color_down
 cmp r4, #2
 bne event_done
 cmp r0, #6
 bhs event_done
 adds r0, #1
 b color_set
color_down:
 cmp r0, #0
 beq event_done
 subs r0, #1
color_set:
 cmp r0, r5
 beq event_done
 adr r1,choice_codes
 ldrh r0,[r1,r0,lsl #1]
 ldr r1, =0x200089d6
 strh r0, [r1]
 ldr r1, =0x200061a0
 movs r0, #10
 str r0, [r1]
 /* Force stock preset/looper LED caches to refresh on the next update. */
 ldr r1, =0x20006178
 movw r0, #0xffff
 strh r0, [r1]
 strh r0, [r1, #2]
 ldr r1, =0x20006159
 movs r0, #1
 strb r0, [r1]
 call 0x19638
 b event_redraw
time_event:
 call 0x16670
 cmp r4, #1
 beq time_down
 cmp r4, #2
 bne event_done
 cmp r0, #8
 bhs event_done
 adds r0, #1
 b time_set
time_down:
 cmp r0, #0
 beq event_done
 subs r0, #1
time_set:
 call 0x28a68
event_redraw:
 ldr r1, =0x20005c70
 movs r0, #1
 strb r0, [r1, #5]
event_done:
 pop {r4-r6, pc}
.ltorg

/* Draw with stock text widgets and stock selectors. Page 1 is untouched. */
.global display_hook
.thumb_func
display_hook:
 bl draw_page
 jump 0x17a70
.ltorg

.thumb_func
widget:
 ldr r0, =0x20005ca0
 ldr r0, [r0]
 jump 0x322c0
.ltorg

.thumb_func
style_value:
 push {r4, lr}
 mov r4, r0
 movs r0, #1
 call 0x28f68
 mov r2, r0
 movs r1, #0
 mov r0, r4
 call 0x20222
 pop {r4, pc}
.ltorg

.thumb_func
draw_page:
 push {r4-r6, lr}
 movw r1, #0x811
 bl widget
 adr r1, button_label
 call 0x2b35e
 movw r1, #0x814
 bl widget
 mov r4, r0
 movs r1, #0
 movs r2, #6
 call 0x200c4
 bl choice
 mov r5, r0
 mov r1, r0
 mov r0, r4
 call 0x20280
 adr r1, color_names
 ldr r1, [r1, r5, lsl #2]
 mov r0, r4
 call 0x20174
 mov r0, r4
 bl style_value

 movw r1, #0x812
 bl widget
 mov r4, r0
 movs r0, #0xdd
 call 0x1bcdc
 mov r1, r0
 mov r0, r4
 call 0x2b35e
 movw r1, #0x815
 bl widget
 mov r4, r0
 movs r1, #0
 movs r2, #8
 call 0x200c4
 call 0x16670
 cmp r0, #8
 it hi
 movhi r0, #0
 mov r5, r0
 mov r1, r0
 mov r0, r4
 call 0x20280
 adr r1, time_names
 ldr r1, [r1, r5, lsl #2]
 mov r0, r4
 call 0x20174
 mov r0, r4
 bl style_value

 movw r1, #0x813
 bl widget
 adr r1, empty
 call 0x2b35e
 movw r1, #0x816
 bl widget
 mov r4, r0
 movs r1, #0
 call 0x20280
 mov r0, r4
 adr r1, empty
 call 0x20174
 pop {r4-r6, pc}
.ltorg
.balign 4
button_label: .asciz "BUTTON COLOR"
gold: .asciz "Gold"
red: .asciz "Red"
blue: .asciz "Blue"
green: .asciz "Green"
orange: .asciz "Orange"
cyan: .asciz "Cyan"
white: .asciz "White"
purple: .asciz "Purple"
always: .asciz "Always On"
t1: .asciz "1Min"
t5: .asciz "5Min"
t10: .asciz "10Min"
t20: .asciz "20Min"
t30: .asciz "30Min"
t40: .asciz "40Min"
t50: .asciz "50Min"
t60: .asciz "60Min"
empty: .asciz ""
.balign 4
color_names: .word gold,red,blue,purple,cyan,green,orange
choice_codes: .hword 0xbc03,0xbc04,0xbc05,0xbc02,0xbc00,0xbc06,0xbc07
.balign 4
time_names: .word always,t1,t5,t10,t20,t30,t40,t50,t60

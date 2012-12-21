@.section .iwram,"ax",%progbits
.arm
.align 4
.global postProcess
.global stackEnd
.type postProcess, %function

@ r0 = screen pointer
@ r1 = stack pointer
@ r2-r9 = data
@ r10 = yellow
@ r11 = cyan
@ r12 = max addr


postProcess:

  @BKPT
  stmfd sp!, {r0-r12}
  mov r12, #192*256*2
  add r12, r12, r0
  ldr r10, orange
  ldr r11, cyan  
  b .loop
  
 .loop_1_2:
	sub r0, r0, #8*4
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	add r0, r0, #8*4
	b .loop_00
  
 .loop_2_2:
	sub r0, r0, #7*4
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	add r0, r0, #7*4
	b .loop_11
  
 .loop_3_2:
	sub r0, r0, #6*4
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	add r0, r0, #6*4
	b .loop_22
  
 .loop_4_2:
	sub r0, r0, #5*4
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	add r0, r0, #5*4
	b .loop_33
  
 .loop_5_2:
	sub r0, r0, #4*4
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	add r0, r0, #4*4
	b .loop_44
  
 .loop_6_2:
	sub r0, r0, #3*4
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	add r0, r0, #3*4
	b .loop_55
  
 .loop_7_2:
	sub r0, r0, #2*4
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	add r0, r0, #2*4
	b .loop_66
  
 .loop_8_2:
	sub r0, r0, #1*4
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	add r0, r0, #1*4
	b .loop_77
  
 .loop_1_3:
	sub r0, r0, #8*4
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	add r0, r0, #8*4
	b .loop_00
  
 .loop_2_3:
	sub r0, r0, #7*4
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	add r0, r0, #7*4
	b .loop_11
  
 .loop_3_3:
	sub r0, r0, #6*4
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	add r0, r0, #6*4
	b .loop_22
  
 .loop_4_3:
	sub r0, r0, #5*4
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	add r0, r0, #5*4
	b .loop_33
  
 .loop_5_3:
	sub r0, r0, #4*4
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	add r0, r0, #4*4
	b .loop_44
  
 .loop_6_3:
	sub r0, r0, #3*4
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	add r0, r0, #3*4
	b .loop_55
  
 .loop_7_3:
	sub r0, r0, #2*4
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	add r0, r0, #2*4
	b .loop_66
  
 .loop_8_3:
	sub r0, r0, #1*4
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	add r0, r0, #1*4
	b .loop_77
  
 .loop:
  @load current pixels
  ldmia  r0!, {r2-r9}
  
  .loop_00:
	  cmp r2, r10
	  beq .loop2_1
	  cmp r2, r11
	  beq .loop3_1
  
  .loop_11:
	  cmp r3, r10
	  beq .loop2_2
	  cmp r3, r11
	  beq .loop3_2
  
  .loop_22:
	  cmp r4, r10
	  beq .loop2_3
	  cmp r4, r11
	  beq .loop3_3
  
  .loop_33:
	  cmp r5, r10
	  beq .loop2_4
	  cmp r5, r11
	  beq .loop3_4
  
  .loop_44:
	  cmp r6, r10
	  beq .loop2_5
	  cmp r6, r11
	  beq .loop3_5
  
  .loop_55:
	  cmp r7, r10
	  beq .loop2_6
	  cmp r7, r11
	  beq .loop3_6
  
  .loop_66:
	  cmp r8, r10
	  beq .loop2_7
	  cmp r8, r11
	  beq .loop3_7
  
  .loop_77:
	  cmp r9, r10
	  beq .loop2_8
	  cmp r9, r11
	  beq .loop3_8
	  
  cmp r0, r12
  blt .loop
  b .end1
  
 .loop2_1:
	sub r0, r0, #8*4
		str r0, [r1], #4
	add r0, r0, #8*4
	b .loop2_11
  
 .loop2_2:
	sub r0, r0, #7*4
		str r0, [r1], #4
	add r0, r0, #7*4
	b .loop2_22
  
 .loop2_3:
	sub r0, r0, #6*4
		str r0, [r1], #4
	add r0, r0, #6*4
	b .loop2_33
  
 .loop2_4:
	sub r0, r0, #5*4
		str r0, [r1], #4
	add r0, r0, #5*4
	b .loop2_44
  
 .loop2_5:
	sub r0, r0, #4*4
		str r0, [r1], #4
	add r0, r0, #4*4
	b .loop2_55
  
 .loop2_6:
	sub r0, r0, #3*4
		str r0, [r1], #4
	add r0, r0, #3*4
	b .loop2_66
  
 .loop2_7:
	sub r0, r0, #2*4
		str r0, [r1], #4
	add r0, r0, #2*4
	b .loop2_77
  
 .loop2_8:
	sub r0, r0, #1*4
		str r0, [r1], #4
	add r0, r0, #1*4
	b .loop2
 
 .loop2:
  @load current pixel (first of couple)
  ldmia  r0!, {r2-r9}
  @BKPT
  
	  cmp r2, r10
	  bne .loop_1_2
  
 .loop2_11:
	  cmp r3, r10
	  bne .loop_2_2
  
 .loop2_22:
	  cmp r4, r10
	  bne .loop_3_2
  
 .loop2_33:
	  cmp r5, r10
	  bne .loop_4_2
  
 .loop2_44:
	  cmp r6, r10
	  bne .loop_5_2
  
 .loop2_55:
	  cmp r7, r10
	  bne .loop_6_2
  
 .loop2_66:
	  cmp r8, r10
	  bne .loop_7_2
  
 .loop2_77:
	  cmp r9, r10
	  bne .loop_8_2
	  
  cmp r0, r12
  blt .loop2
  b .end2
  
 .loop3_1:
	sub r0, r0, #8*4
		str r0, [r1], #4
	add r0, r0, #8*4
	b .loop3_11
  
 .loop3_2:
	sub r0, r0, #7*4
		str r0, [r1], #4
	add r0, r0, #7*4
	b .loop3_22
  
 .loop3_3:
	sub r0, r0, #6*4
		str r0, [r1], #4
	add r0, r0, #6*4
	b .loop3_33
  
 .loop3_4:
	sub r0, r0, #5*4
		str r0, [r1], #4
	add r0, r0, #5*4
	b .loop3_44
  
 .loop3_5:
	sub r0, r0, #4*4
		str r0, [r1], #4
	add r0, r0, #4*4
	b .loop3_55
  
 .loop3_6:
	sub r0, r0, #3*4
		str r0, [r1], #4
	add r0, r0, #3*4
	b .loop3_66
  
 .loop3_7:
	sub r0, r0, #2*4
		str r0, [r1], #4
	add r0, r0, #2*4
	b .loop3_77
  
 .loop3_8:
	sub r0, r0, #1*4
		str r0, [r1], #4
	add r0, r0, #1*4
	b .loop3
 
.loop3:
  @load current pixel (first of couple)
  ldmia  r0!, {r2-r9}
  
	  cmp r2, r11
	  bne .loop_1_3
  
  .loop3_11:
	  cmp r3, r11
	  bne .loop_2_3
  
  .loop3_22:
	  cmp r4, r11
	  bne .loop_3_3
  
  .loop3_33:
	  cmp r5, r11
	  bne .loop_4_3
  
  .loop3_44:
	  cmp r6, r11
	  bne .loop_5_3
  
  .loop3_55:
	  cmp r7, r11
	  bne .loop_6_3
  
  .loop3_66:
	  cmp r8, r11
	  bne .loop_7_3
  
  .loop3_77:
	  cmp r9, r11
	  bne .loop_8_3
	  
  cmp r0, r12
  blt .loop3
  b .end3

.end1:
  str r12, [r1], #4
  b .end

.end2:
	add r0, r0, #256*192*2
		str r0, [r1], #4
	sub r0, r0, #256*192*2
	b .end

.end3:
	add r0, r0, #256*192*4
		str r0, [r1], #4
	sub r0, r0, #256*192*4
	b .end
  
.end:  
  @return...
  str r1, stackEnd
  ldmfd sp!, {r0-r12}
  bx    lr
  .align 4
  .size postProcess, .-postProcess
  
orange1:
	.hword 33758
	.hword 33758
cyan1:
	.hword 64448
	.hword 64448
orange:
	.hword 33791
	.hword 33791
cyan:
	.hword 65504
	.hword 65504
stackEnd:
	.word 0
.align 4

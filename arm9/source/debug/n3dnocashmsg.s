@ ------------------------------------------------------------------------------
@
@ This function can be used to output No$gba debug messages.
@
@ Author: Peter Schraut (www.console-dev.de)
@ Date..: 2005-Jan-20
@
@ extern "C" int N3DNoCashMsg(const char *pText);
@
@ ------------------------------------------------------------------------------
@
@ Copyright 2005-2007 by Peter Schraut www.console-dev.de
@ 
@ This file is distributed as freeware.
@ 
@ You are free to use it as part of your program for any purpose including 
@ freeware, shareware and commercial programs.
@ 
@ The origin of this software must not be misrepresented; you must not claim your 
@ authorship. All redistributions must retain the original copyright notice and web 
@ site addresses.
@ 
@ Commercial redistribution of this file is allowed only with an explicit written
@ permission from the author.
@ 
@ This software is provided 'as-is', without warranty of any kind, either expressed or 
@ implied. In no event shall the author be held liable for any damages arising from the 
@ use of this software.
@
@ ------------------------------------------------------------------------------

@.section .iwram,"ax",%progbits
.arm
.align 4
.global N3DNoCashMsg
.type N3DNoCashMsg, %function

@ r0 = Text
@ r1 = Address of target buffer
@ r2 = Maximum amount of characters, descrements each iteration
@ r3 = Current char of text
.equiv  BUFFER_SIZE, 120

N3DNoCashMsg:

  stmfd sp!, {r1-r3}

  adr   r1, .Buffer       @ Get target buffer address
  mov   r2, #BUFFER_SIZE  @ Maximum amount of supported characters

@ This loop copies the incoming data into the
@ Buffer below. If it encounters a NULL-terminator
@ or is beyond the 120th character, it will break.
.Loop:
  ldrb  r3, [r0], #1      @ Load current byte from text
  strb  r3, [r1], #1      @ Store currrent byte into Buffer

  subs  r2, r2, #1        @ One further byte added
  cmpne r3, #0            @ Came across a NULL-terminator?
  beq   .MsgTag           @ Last Char or NULL-terminator? Then branch to MsgTag
  b     .Loop             @ Just loop

@ Here starts the actual no$gba Message Tag
.MsgTag:

  mov   r12, r12          @ First no$gba ID
  b     .Continue

  .short 0x6464           @ Second no$gba ID
  .short 0                @ Reserved for flags

.Buffer:
  .space BUFFER_SIZE      @ 120 bytes for the message
  .byte  0                @ Trailing zero as NULL-Terminator
  .align 4

.Continue:
	ldr   r0, =(BUFFER_SIZE-1)	@ Return length of Text without NULL-terminator
	sub   r0, r0, r2

  ldmfd sp!, {r1-r3}
  bx    lr
  .align 4
  .size N3DNoCashMsg, .-N3DNoCashMsg


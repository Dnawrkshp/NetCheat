
/*
NetCheat engine
Written by Dnawrkshp
*/

address $00047010
//address $00060010

_init:
addiu sp, sp, $FFE0
sq ra, $0000(sp)
sq v0, $0010(sp)

jal :_start
nop

lui v0, $8000
ori v0, v0, $0234 //Hooked with a jalr k0, else with a jr k0
lq ra, $0000(sp)
beq ra, v0, 3
lq v0, $0010(sp)
jr k0
addiu sp, sp, $0020

jalr k0
addiu sp, sp, $0020

_start:
addiu sp, sp, $FE00
sq at, $0000(sp)
sq v0, $0010(sp)
sq v1, $0020(sp)
sq a0, $0030(sp)
sq a1, $0040(sp)
sq a2, $0050(sp)
sq a3, $0060(sp)
sq t0, $0070(sp)
sq t1, $0080(sp)
sq t2, $0090(sp)
sq t3, $00a0(sp)
sq t4, $00b0(sp)
sq t5, $00c0(sp)
sq t6, $00d0(sp)
sq t7, $00e0(sp)
sq s0, $00f0(sp)
sq s1, $0100(sp)
sq s2, $0110(sp)
sq s3, $0120(sp)
sq s4, $0130(sp)
sq s5, $0140(sp)
sq s6, $0150(sp)
sq s7, $0160(sp)
sq t8, $0170(sp)
sq t9, $0180(sp)
sq k0, $0190(sp)
sq k1, $01a0(sp)
sq fp, $01b0(sp)
sq gp, $01c0(sp)
sq ra, $01d0(sp)

//Location of pointer to current code - based around address of Engine
setreg k0, :_init
addiu k0, k0, $FFF0
lui at, $8000
daddu k0, at, k0

//Constant, used for incrementing each code
addiu at, zero, $0008

//Check if a process of this is already going
//If it is, exit. Credit to Gtlcpimp
//CL-LiveDebug doesn't like this... so just to check whether in DEBUG mode
lui v0, $8000
ori v0, v0, $0234 //Hooked with a jalr k0 (DEBUG mode), else with a jr k0
beq ra, v0, 6
nop

lb t0, $FFFC(k0)
bne t0, zero, :_NCExitProcInProg
nop
addiu t0, zero, 1
sb t0, $FFFC(k0)

//Look for mastercode (9 type)
//If it's not there then continue
lw t1, $0000(k0)
lw t0, $FFF8(t1) //Mastercode address
lw t1, $FFFC(t1) //Mastercode value

beq t0, zero, 4 //No mastercode
nop
lw t0, $0000(t0)
bne t0, t1, :_NCExit
nop

_NCLoop:
lw t1, $0000(k0)
lw t2, $0004(k0) //Counter
lw t0, $0000(t1)

addiu v0, zero, 128 //Max counter
slt v0, v0, t2
bne v0, zero, :_NCExitNotDone
nop

beq t0, zero, :_NCExit
nop

jal :_code
daddu a0, t1, zero

beq zero, zero, :_NCLoop
nop

_NCExit: //NetCheat Exit and Reset
lw t0, $FFF8(k0) //Initial pointer
sw t0, $0000(k0)

_NCExitNotDone: //NetCheat Exit (More Codes Left, 256 max limit reached)
sw zero, $0004(k0)
sw zero, $FFFC(k0)

_NCExitProcInProg:
lq at, $0000(sp)
lq v0, $0010(sp)
lq v1, $0020(sp)
lq a0, $0030(sp)
lq a1, $0040(sp)
lq a2, $0050(sp)
lq a3, $0060(sp)
lq t0, $0070(sp)
lq t1, $0080(sp)
lq t2, $0090(sp)
lq t3, $00a0(sp)
lq t4, $00b0(sp)
lq t5, $00c0(sp)
lq t6, $00d0(sp)
lq t7, $00e0(sp)
lq s0, $00f0(sp)
lq s1, $0100(sp)
lq s2, $0110(sp)
lq s3, $0120(sp)
lq s4, $0130(sp)
lq s5, $0140(sp)
lq s6, $0150(sp)
lq s7, $0160(sp)
lq t8, $0170(sp)
lq t9, $0180(sp)
lq k0, $0190(sp)
lq k1, $01a0(sp)
lq fp, $01b0(sp)
lq gp, $01c0(sp)
lq ra, $01d0(sp)
jr ra
addiu sp, sp, $0200

_code:
addiu sp, sp, $FEF0
sq ra, $0000(sp)
sq t0, $0010(sp)
sq t1, $0020(sp)
sq t2, $0030(sp)
sq t3, $0040(sp)
sq t4, $0050(sp)
sq t5, $0060(sp)
sq t6, $0070(sp)
sq t7, $0080(sp)
sq t8, $0090(sp)
sq t9, $00A0(sp)
sq s0, $00B0(sp)
sq s1, $00C0(sp)
sq s2, $00D0(sp)
sq s3, $00E0(sp)
sq k1, $00F0(sp)
sq s7, $0100(sp)

daddu s7, a0, zero //pointer to code

lw a0, $0000(s7) //address
beq a0, zero, :_exit
nop
lw a1, $0004(s7) //value

srl a2, a0, 28
srl a3, a0, 24
sll t0, a2, 28
subu a0, a0, t0

jal :_procCodes
nop

_exit:

daddu v0, s7, zero //returns pointer to current address (for use with E codetype)
lq ra, $0000(sp)
lq t0, $0010(sp)
lq t1, $0020(sp)
lq t2, $0030(sp)
lq t3, $0040(sp)
lq t4, $0050(sp)
lq t5, $0060(sp)
lq t6, $0070(sp)
lq t7, $0080(sp)
lq t8, $0090(sp)
lq t9, $00A0(sp)
lq s0, $00B0(sp)
lq s1, $00C0(sp)
lq s2, $00D0(sp)
lq s3, $00E0(sp)
lq k1, $00F0(sp)
lq s7, $0100(sp)
jr ra
addiu sp, sp, $0110

_procCodes:
addiu sp, sp, $FFA0
sq ra, $0000(sp)
sq s0, $0010(sp)
sq s1, $0020(sp)
sq s2, $0030(sp)
sq s3, $0040(sp)
sq s4, $0050(sp)

daddu s0, a0, zero //addr
daddu s1, a1, zero //value
daddu s2, a2, zero //4 bit command
daddu s3, a3, zero //8 bit command

addiu v0, zero, $000B
beq s2, v0, 3
nop
beq s0, zero, :_ERROR
nop

//8-bit write
bne s2, zero, 3
nop
sb s1, $0000(s0)
beq zero, zero, :_ERROR

//16-bit write
addiu t0, zero, $0001
bne s2, t0, 3
nop
sh s1, $0000(s0)
beq zero, zero, :_ERROR

//32-bit write
addiu t0, zero, $0002
bne s2, t0, 3
nop
sw s1, $0000(s0)
beq zero, zero, :_ERROR

//Inc/Dec
addiu t0, zero, $0003
bne s2, t0, 4
nop
jal :_3
nop
beq zero, zero, :_ERROR

//Multi-address write
addiu t0, zero, $0004
bne s2, t0, 4
nop
jal :_4
nop
beq zero, zero, :_ERROR

//Copy bytes
addiu t0, zero, $0005
bne s2, t0, 4
nop
jal :_5
nop
beq zero, zero, :_ERROR

//Pointer
addiu t0, zero, $0006
bne s2, t0, 4
nop
jal :_6
nop
beq zero, zero, :_ERROR

//Bitwise operate and write
addiu t0, zero, $0007
bne s2, t0, 4
nop
jal :_7
nop
beq zero, zero, :_ERROR

//Master conditional (KSEG0)
addiu t0, zero, $0008
bne s2, t0, 4
nop
jal :_8
nop
beq zero, zero, :_ERROR

//Master write (KSEG0)
addiu t0, zero, $000A
bne s2, t0, 4
nop
jal :_A
nop
beq zero, zero, :_ERROR

//Delay
addiu t0, zero, $000B
bne s2, t0, 4
nop
jal :_B
nop
beq zero, zero, :_ERROR

//32-bit joker
addiu t0, zero, $000C
bne s2, t0, 4
nop
jal :_C
nop
beq zero, zero, :_ERROR

//16-bit joker
addiu t0, zero, $000D
bne s2, t0, 4
nop
jal :_D
nop
beq zero, zero, :_ERROR

//multi-line 16-bit joker
addiu t0, zero, $000E
bne s2, t0, 4
nop
jal :_E
nop
beq zero, zero, :_ERROR

//Hook
addiu t0, zero, $00F1
bne s3, t0, 4
nop
jal :_F1
daddu a1, s1, zero
beq zero, zero, :_ERROR

//Switch conditional
addiu t0, zero, $00F2
bne s3, t0, 4
nop
jal :_F2
daddu a1, s1, zero
beq zero, zero, :_ERROR

nop
//Unsupported command or done processing code
_ERROR:
daddu s7, s7, at
sw s7, $0000(k0)
lw t1, $0004(k0)
addiu t1, t1, 1
sw t1, $0004(k0)

lq ra, $0000(sp)
lq s0, $0010(sp)
lq s1, $0020(sp)
lq s2, $0030(sp)
lq s3, $0040(sp)
lq s4, $0050(sp)
jr ra
addiu sp, sp, $0060

//==================================================
/*
Increment and Decrement (switchable)

8 and 16 bit:
30w0xxxx 0aaaaaaa
w: Switch (0 = 8-bit Inc, 1 = 8-bit Dec, 2 = 16-bit Inc, 3 = 16-bit Dec)
x: 16-bit value
a: Address

32 bit:
30w00000 0aaaaaaa
xxxxxxxx 00000000
w: Switch (4 = 32-bit Inc, 5 = 32-bit Dec)
x: 32-bit value
a: Address

Example:
30500000 0133FC54
00100000 00000000
*/
_3:

sll t0, s0, 8
srl t0, t0, 28 //Switch

bne t0, zero, :_3Not0
nop
//8-bit increment
sll t0, s0, 24
srl t0, t0, 24 //8-bit value

lb t1, $0000(s1)
addu t1, t1, t0
sb t1, $0000(s1)

beq zero, zero, :_3Exit
nop

_3Not0:
addiu v0, zero, 1
bne t0, v0, :_3Not1
nop
//8-bit decrement
sll t0, s0, 24
srl t0, t0, 24 //8-bit value

lb t1, $0000(s1)
subu t1, t1, t0
sb t1, $0000(s1)

beq zero, zero, :_3Exit
nop

_3Not1:
addiu v0, zero, 2
bne t0, v0, :_3Not2
nop
//16-bit increment
sll t0, s0, 16
srl t0, t0, 16 //16-bit value

lh t1, $0000(s1)
addu t1, t1, t0
sh t1, $0000(s1)

beq zero, zero, :_3Exit
nop

_3Not2:
addiu v0, zero, 3
bne t0, v0, :_3Not3
nop
//16-bit increment
sll t0, s0, 16
srl t0, t0, 16 //16-bit value

lh t1, $0000(s1)
subu t1, t1, t0
sh t1, $0000(s1)

beq zero, zero, :_3Exit
nop

_3Not3:
addiu v0, zero, 4
bne t0, v0, :_3Not4
nop
//32-bit increment
lw t0, $0008(s7) //Next address (value to increment with)

lw t1, $0000(s1)
daddu t1, t1, t0
sw t1, $0000(s1)

beq zero, zero, :_3Exit
daddu s7, s7, at

_3Not4:
addiu v0, zero, 5
bne t0, v0, :_3Exit
nop
//32-bit decrement
lw t0, $0008(s7) //Next address (value to decrement with)

lw t1, $0000(s1)
dsubu t1, t1, t0
sw t1, $0000(s1)

beq zero, zero, :_3Exit
daddu s7, s7, at

_3Exit:
jr ra
nop

//==================================================
/*
Multi-address Write (slide-fill code)
4aaaaaaa wcccssss
xxxxxxxx iiiiiiii

a: 25-bit address.
w: width (0 = 32bit, 1 = 16bit, 2 = 8bit)
c: Number of times to write (count).
s: Step value.
x: 32-bit value.
i: 32-bit value increment.

Example:
40EE7174 00060002
00000063 00000001
*/
_4:
addiu sp, sp, $FFB0
sq s1, $0000(sp)
sq s2, $0010(sp)
sq s3, $0020(sp)
sq s4, $0030(sp)
sq s5, $0040(sp)

daddu s7, s7, at
beq s0, zero, :_4Exit //Address is zero: exit
nop

//Address
daddu t0, s0, zero

//Count
sll s2, s1, 4
srl s2, s2, 20

//Step
sll s3, s1, 16
srl s3, s3, 16

//value
lw s4, $0000(s7)
//value increment
lw s5, $0004(s7)
//width
srl s1, s1, 28

//32 bit align (multiply by 4)
bne s1, zero, 3
nop
addiu v0, zero, 4
multu s3, s3, v0

//16 bit align (multiply by 2)
addiu v0, zero, 1
bne s1, v0, 3
nop
addiu v0, zero, 4
multu s3, s3, v0

_4Loop:
//When counter is zero: exit
beq s2, zero, :_4Exit
nop

//32bit write
bne s1, zero, 2
nop
sw s4, $0000(t0)

//16bit write
addiu v0, zero, 1
bne s1, v0, 2
nop
sh s4, $0000(t0)

//8bit write
addiu v0, zero, 2
bne s1, v0, 2
nop
sb s4, $0000(t0)

daddu t0, t0, s3
daddu s4, s4, s5

beq zero, zero, :_4Loop
addiu s2, s2, -1

_4Exit:
lq s1, $0000(sp)
lq s2, $0010(sp)
lq s3, $0020(sp)
lq s4, $0030(sp)
lq s5, $0040(sp)
jr ra
addiu sp, sp, $0050

//==================================================
/*
Copy bytes
5aaaaaaa cccccccc
0ddddddd 00000000

a: 25-bit source address.
c: number of bytes to copy.
d: 25-bit destination address.

Example:
50EE7175 00000064
00EB00B5 00000000
*/
_5:
addiu sp, sp, $FFE0
sq s0, $0000(sp)
sq s1, $0010(sp)

daddu s7, s7, at
lw t0, $0000(s7) //Destination

sw s1, $FFF0(k0)

beq s0, zero, :_5Exit
nop

beq t0, zero, :_5Exit
nop

_5Loop:
beq s1, zero, :_5Exit
nop

lb t1, $0000(s0)
sb t1, $0000(t0)

addiu s0, s0, 1
addiu t0, t0, 1
beq zero, zero, :_5Loop
addiu s1, s1, -1

_5Exit:
lq s0, $0000(sp)
lq s1, $0010(sp)
jr ra
addiu sp, sp, $0020

//==================================================
/*
Pointer code
6aaaaaaa vvvvvvvv
000wnnnn oooooooo

a = address, v = value, w = store type (0 = byte, 1 = half, 2 = word), n = number of layers, o = offset

Example:
601034BC 00000001
00020003 0000011C //Off 1
00000011 (2) 00000110 (3) //Off 2 and Off 3

NOTE: If the pointer points to NULL (0) it will exit!
*/

_6:
addiu sp, sp, $FFE0
sq ra, $0000(sp)
sq k1, $0010(sp)

daddu k1, s7, at
lw t1, $0000(k1) //Load w, and n
sll t2, t1, 16
srl t2, t2, 16 //Get n
srl t1, t1, 16 //Get w

//calculate number of addresses after codes
addiu t7, t2, 1 //Account for the 000wnnnn oooooooo
sll t6, t7, 31

beq t6, zero, 2 //If zero then it is even and alignment is a-okay
nop
addiu t7, t7, 1

addiu v0, zero, 4
multu t7, t7, v0

lui a1, $0008
lui a2, $0200
jal :_IfBetween
lw a0, $0000(s0)

bne v0, zero, :_6Exit //If it's not in the EE, then exit
nop

beq t2, zero, :_6Exit
nop

daddu t5, s0, zero //Transfer address to t5

_6Loop:
lw t0, $0000(t5) //Load address
lw t4, $0004(k1) //Load offset
addu t5, t0, t4  //Offsetted address

addiu t2, t2, -1
bne t2, zero, :_6Loop
addiu k1, k1, 4

beq t5, zero, :_6Exit
nop

//Store value at final address
bne t1, zero, 2 //8 bit
nop
sb s1, $0000(t5)

addiu v0, zero, 1 //16 bit
bne t1, v0, 2
nop
sh s1, $0000(t5)

addiu v0, zero, 2 //32 bit
bne t1, v0, 2
nop
sw s1, $0000(t5)

_6Exit:
daddu s7, s7, t7 //New code address

lq ra, $0000(sp)
lq k1, $0010(sp)
jr ra
addiu sp, sp, $0020

//==================================================
/*
Bitwise operate and write

7aaaaaaa 00t0vvvv
a - Address to be operated on.
v - Mask value to use in operation.
t - Operation type.

Values for t:
0 - Bitwise OR the byte at a with the value v and store at a.
1 - Bitwise OR the half-word at a with the value v and store at a.
2 - Bitwise AND the byte at a with the value v and store at a.
3 - Bitwise AND the half-word at a with the value v and store at a.
4 - Bitwise XOR the byte at a with the value v and store at a.
5 - Bitwise XOR the half-word at a with the value v and store at a.

Example:
701FFC10 00101234
*/
_7:

//Operation
sll t0, s1, 8
srl t0, t0, 28

//Mask value
sll t1, s1, 16
srl t1, t1, 16

beq s0, zero, :_7Exit
nop

bne t0, zero, 4
nop
lb t2, $0000(s0)
or t2, t2, t1
sb t2, $0000(s0)

addiu v0, zero, 1
bne t0, v0, 4
nop
lh t2, $0000(s0)
or t2, t2, t1
sh t2, $0000(s0)

addiu v0, zero, 2
bne t0, v0, 4
nop
lb t2, $0000(s0)
and t2, t2, t1
sb t2, $0000(s0)

addiu v0, zero, 3
bne t0, v0, 4
nop
lh t2, $0000(s0)
and t2, t2, t1
sh t2, $0000(s0)

addiu v0, zero, 4
bne t0, v0, 4
nop
lb t2, $0000(s0)
xor t2, t2, t1
sb t2, $0000(s0)

addiu v0, zero, 5
bne t0, v0, 4
nop
lh t2, $0000(s0)
xor t2, t2, t1
sh t2, $0000(s0)

_7Exit:
jr ra
nop

//==================================================
/*
Master conditional
8aaaaaaa ccccxxxx

a: 32-bit address (the command 8 is part of the address).
c: Number of code-lines to skip (count).
x: 16-bit comparison value.

Example:
80080A28 00010004
A008069C 08080000
*/
_8:
addiu sp, sp, $FFC0
sq ra, $0000(sp)
sq s2, $0010(sp)
sq s3, $0020(sp)
sq s4, $0030(sp)

//count
srl s2, s1, 16

//value
sll s3, s1, 16
srl s3, s3, 16

multu s4, s2, at

beq s4, zero, :_8Exit
nop

//Condition check
lhu t0, $0000(s0)
bne s3, t0, :_8Exit
nop

//Point address to KSEG0
lui t0, $8000
daddu t0, t0, s0

daddu s3, s7, at
daddu s2, s3, s4

//beq zero, zero, :_8Exit
nop

_8Loop:
beq s3, s2, :_8Exit
nop

lw t0, $0000(s3)
beq t0, zero, :_8Exit
nop

jal :_code
daddu a0, s3, zero

beq zero, zero, :_8Loop
daddu s3, v0, zero

_8Exit:
daddu s7, s7, s4

lq ra, $0000(sp)
lq s2, $0010(sp)
lq s3, $0020(sp)
lq s4, $0030(sp)
jr ra
addiu sp, sp, $0040

//==================================================
/*
Master write
Aaaaaaaa xxxxxxxx

a: 25-bit address.
x 32-bit comparison value.

Example:
A0EE1008 1007FFFC
*/
_A:
beq s0, zero, :_AExit
nop

lui t0, $8000
daddu t0, t0, s0
sw s1, $0000(t0)

//I don't understand why you need to write to this
/*
lui t0, $2000
daddu t0, t0, s0
sw s1, $0000(t0)
*/

_AExit:
jr ra
nop

//==================================================
/*
Delay
B0000000 0xxxxxxx

x: 28-bit value.

NOTES:
I removed the switch option because only 0 seemed like it served a purpose
The delay is only 28 bits so I can fit the the counter in the 28 bits of the address

Example:
B0000000 01000000
*/
_B:
addiu sp, sp, $FFE0
sq s0, $0000(sp)
sq s1, $0010(sp)

sll s1, s1, 4
srl s1, s1, 4
slt t0, s0, s1

beq t0, zero, :_BExit
nop
//Skip lines below
addiu s0, s0, 1
sll s0, s0, 4
srl s0, s0, 4

lui t0, $B000
or t0, t0, s0
//Store increment
sw t0, $0000(s7)

//Skip the rest by setting the amount of codes done to the max
addiu v0, zero, 128
sw v0, $0004(k0)

beq zero, zero, :_BExitSkip
daddu s7, k0, at //Reset the pointer to the current code (procCodes will add 8, so set it to 8 less

_BExit:
//Reset counter
lui t0, $B000
sw t0, $0000(s7)

_BExitSkip:
lq s0, $0000(sp)
lq s1, $0010(sp)
jr ra
addiu sp, sp, $0020

//==================================================
/*
32-bit conditional

Caaaaaaa vvvvvvvv

a = address
v = value

If value at address = value execute CODE

Example:
C010022C 03E00008
2012715C 08004000
*/
_C:
addiu sp, sp, $FFF0
sq ra, $0000(sp)

lw t0, $0000(s0) //Get value

bne t0, s1, :_CExit
nop

jal :_code
daddu a0, s7, at

_CExit:
daddu s7, s7, at
lq ra, $0000(sp)
jr ra
addiu sp, sp, $0010

//==================================================
/*
16-bit conditional (joker)

Daaaaaaa 00twvvvv

a = address
t = type (3 bit)
w = width (1 bit, 1 = 16, 0 = 8 bit)
v = value

If value at address = value execute CODE

Example:
D01EE682 0000FFF9
2012715C 08004000
*/

_D:
addiu sp, sp, $FFF0
sq ra, $0000(sp)

sll a2, s1, 8
srl a2, a2, 28 //type
sll a1, s1, 16
srl a1, a1, 16 //value
sll a3, s1, 14
srl a3, a3, 30 //width
daddu s3, zero, zero //Counter

jal :_ParseJokerType
daddu a0, s0, zero

beq v0, zero, :_DExit
nop

jal :_code
daddu a0, s7, at

_DExit:
daddu s7, s7, at
lq ra, $0000(sp)
jr ra
addiu sp, sp, $0010

//==================================================
/*
Multi-line conditional (joker)

Ewnnvvvv taaaaaaa

w = width (0 = 16, 1 = 8 bit)
n = number of lines under it
v = value
t = type
a = address

If value at address = value execute CODE

Example:
E002FFF9 401EE682
2012715C 08004000
20127160 00000000
*/
_E:
addiu sp, sp, $FFE0
sq ra, $0000(sp)
sq s2, $0010(sp)

sll a2, s1, 1
srl a2, a2, 29 //type
sll a1, s0, 16
srl a1, a1, 16 //value
sll s2, s0, 8
srl s2, s2, 24 //size
srl a3, s0, 24 //width
daddu s3, zero, zero //Counter

sll a0, s1, 7
jal :_ParseJokerType
srl a0, a0, 7

multu s0, s2, at

beq v0, zero, :_EExit
nop

daddu s3, s7, at
daddu s2, s3, s0

_ELoop:
beq s3, s2, :_EExit
nop

lw t0, $0000(s3)
beq t0, zero, :_EExit
nop

jal :_code
daddu a0, s3, zero

beq zero, zero, :_ELoop
daddu s3, v0, zero

_EExit:
daddu s7, s7, s0

lq ra, $0000(sp)
lq s2, $0010(sp)
jr ra
addiu sp, sp, $0020

//==================================================
/*
Switch joker
Example:
F01EE682 FFF9FFF6
20347E8C 00000000
F is the command
001EE682 is the joker address
FFF9 is off combo
FFF6 is on combo
*/
/*
_F:
addiu sp, sp, $FFF0
sq ra, $0000(sp)

//ori t0, zero, $C000 //Offset
//daddu s3, s7, t0 //Get switches address (0x00010000 more than the pointer to the code)
daddu s3, s7, zero

lb t0, $0003(s3)
sll t0, t0, 6
srl t0, t0, 31

bne t0, zero, 2
nop
//Off - It adds to the counter - effectively skipping the next code
daddu s7, s7, at

lhu t0, $0000(s0) //Get joker

sll t1, s1, 16
srl t1, t1, 16 //On combo
srl t2, s1, 16 //Off combo

bne t0, t1, :_Foff
nop
//On combo
addiu v0, zero, 1
sw v0, $0000(s3)

_Foff:
bne t0, t2, :_FExit
nop
sw zero, $0000(s3)

_FExit:
lq ra, $0000(sp)
jr ra
addiu sp, sp, $0010
*/

//==================================================
/*
Execute Data / Hook (original idea by Gtlcpimp)

F10000dd 0aaaaaaa

a = address
d = delay (address executed every 1 out of d)

Example:
F2000010 000AE000
*/
_F1:
lbu t0, $0001(s7) //Delay
sll t1, s0, 24
srl t1, t1, 24 //8 bit value

bne t0, t1, :_F1Exit
addiu t0, t0, 1

daddu t0, zero, zero //Set back to zero

addiu sp, sp, $FE00
sq at, $0000(sp)
sq v0, $0010(sp)
sq v1, $0020(sp)
sq a0, $0030(sp)
sq a1, $0040(sp)
sq a2, $0050(sp)
sq a3, $0060(sp)
sq t0, $0070(sp)
sq t1, $0080(sp)
sq t2, $0090(sp)
sq t3, $00a0(sp)
sq t4, $00b0(sp)
sq t5, $00c0(sp)
sq t6, $00d0(sp)
sq t7, $00e0(sp)
sq s0, $00f0(sp)
sq s1, $0100(sp)
sq s2, $0110(sp)
sq s3, $0120(sp)
sq s4, $0130(sp)
sq s5, $0140(sp)
sq s6, $0150(sp)
sq s7, $0160(sp)
sq t8, $0170(sp)
sq t9, $0180(sp)
sq k0, $0190(sp)
sq k1, $01a0(sp)
sq fp, $01b0(sp)
sq gp, $01c0(sp)
sq ra, $01d0(sp)

jalr a1
nop

lq at, $0000(sp)
lq v0, $0010(sp)
lq v1, $0020(sp)
lq a0, $0030(sp)
lq a1, $0040(sp)
lq a2, $0050(sp)
lq a3, $0060(sp)
lq t0, $0070(sp)
lq t1, $0080(sp)
lq t2, $0090(sp)
lq t3, $00a0(sp)
lq t4, $00b0(sp)
lq t5, $00c0(sp)
lq t6, $00d0(sp)
lq t7, $00e0(sp)
lq s0, $00f0(sp)
lq s1, $0100(sp)
lq s2, $0110(sp)
lq s3, $0120(sp)
lq s4, $0130(sp)
lq s5, $0140(sp)
lq s6, $0150(sp)
lq s7, $0160(sp)
lq t8, $0170(sp)
lq t9, $0180(sp)
lq k0, $0190(sp)
lq k1, $01a0(sp)
lq fp, $01b0(sp)
lq gp, $01c0(sp)
lq ra, $01d0(sp)
addiu sp, sp, $0200

_F1Exit:
jr ra
sb t0, $0001(s7)

//==================================================
/*
Switch Conditional
F200nnnn taaaaaaa
xxxxxxxx yyyyyyyy

n: number of lines to execute under it if on
t: comparison type (0 = 8 bit, 1 = 16bit, 2 = 32bit)
a: address
x: off value
y: on value

Example:
F2000001 101EE682
0000FFF9 0000FFF6
20347E8C 00000000
*/
_F2:
addiu sp, sp, $FFC0
sq ra, $0000(sp)
sq s1, $0010(sp)
sq s4, $0020(sp)
sq s5, $0030(sp)

daddu s7, s7, at

srl s4, s1, 28 //t
sll t0, s4, 28
subu s1, s1, t0, //a

sll s5, s0, 16
srl s5, s5, 16 //n
multu s5, s5, at
//daddu s5, s7, s5 //next code after this and what follows this

//beq zero, zero, :_F2Exit
nop

sltiu t0, s4, 4
beq t0, zero, :_F2Exit //Unsupported comparison type
nop

bne s4, zero, 4 //8 bit
nop
lbu t0, $0000(s1)
lbu t1, $0000(s7) //off
lbu t2, $0004(s7) //on

addiu v0, zero, $0001
bne s4, v0, 4 //16 bit
nop
lhu t0, $0000(s1)
lhu t1, $0000(s7) //off
lhu t2, $0004(s7) //on

addiu v0, zero, 2
bne s4, v0, 4 //16 bit
nop
lw t0, $0000(s1)
lw t1, $0000(s7) //off
lw t2, $0004(s7) //on

bne t0, t2, :_F2OffCheck
nop
//On
addiu v0, zero, $0010

beq zero, zero, :_F2Exec2
sb v0, $FFFA(s7) //Update value

_F2OffCheck:
bne t0, t1, :_F2Exec
nop
//Off
beq zero, zero, :_F2Exit
sb zero, $FFFA(s7) //Update value

_F2Exec:
sll v0, s0, 8
srl v0, v0, 28 //Get byte bool
beq v0, zero, :_F2Exit
nop
_F2Exec2:

daddu s4, s7, at
daddu s1, s4, s5

_F2Loop:
beq s4, s1, :_F2Exit
nop

lw t0, $0000(s4)
beq t0, zero, :_F2Exit
nop

jal :_code
daddu a0, s4, zero

beq zero, zero, :_F2Loop
daddu s4, v0, zero

_F2Exit:
daddu s7, s7, s5
lq ra, $0000(sp)
lq s1, $0010(sp)
lq s4, $0020(sp)
lq s5, $0030(sp)
jr ra
addiu sp, sp, $0040

//================================================
/*
Finding a free region of memory
Reads the joker from address (s0)
It will output the result to the value you place (s1)
The command can be whatever you desire
Joker trigger is always FFF9 (L3 + R3)

Example:
?01EE682 009FB130

Reads joker from 001EE682
Outputs result as string to 009FB130
*/
/*
_FindR:
addiu sp, sp, $FFF0
sq ra, $0000(sp)

lhu t0, $0000(s0)

addiu v0, zero, $FFF9
bne t0, v0, :_FindRExit
nop
jal :_FindOPL
nop

_FindRExit:

lq ra, $0000(sp)
jr ra
addiu sp, sp, $0010

//========================================================
_FindOPL:
addiu sp, sp, $FFD0
sq s0, $0000(sp)
sq s1, $0010(sp)
sq ra, $0020(sp)

lui a0, $0008
lui a1, $0010
jal :_FindFreeRegion
daddu a2, zero, $2000

daddu a0, s1, zero
daddu a1, v0, zero

jal :_ConvertToString
addiu a2, zero, $0008

_FOExit:
lq s0, $0000(sp)
lq s1, $0010(sp)
lq ra, $0020(sp)
jr ra
addiu sp, sp, $0030

//Convert to string pulled from Socom 2 trainer
_ConvertToString:
addiu sp, sp, $FFD0
sw ra, $0000(sp)
sw s0, $0004(sp)
sw s1, $0008(sp)
sw s2, $000C(sp)
daddu s0, a0, zero
daddu s1, a1, zero
daddu s2, a2, zero
addu s0, s0, s2
//sb zero, $0000(s0)
addiu s0, s0, $FFFF
srl a0, s1, 4
sll a0, a0, 4
subu a0, s1, a0
srl s1, s1, 4
addiu a0, a0, $0030
slti v0, a0, $003A
bne v0, zero, 2
nop
addiu a0, a0, $0007
sb a0, $0000(s0)
addiu s0, s0, $FFFF
addiu s2, s2, $FFFF
bne s2, zero, -13
nop
lw ra, $0000(sp)
lw s0, $0004(sp)
lw s1, $0008(sp)
lw s2, $000C(sp)
jr ra
addiu sp, sp, $0030

//===============================================================
//a0 = Start address
//a1 = Address limit
//a2 = Size of block
//returns v0 as pointer to beginning of memory
_FindFreeRegion:
addiu sp, sp, $FFB0
sq s0, $0000(sp)
sq s1, $0010(sp)
sq s2, $0020(sp)
sq s3, $0030(sp)
sq s4, $0040(sp)

daddu s0, a0, zero //Start addr
daddu s1, a1, zero //Addr limit
daddu s2, a2, zero //Size of block
daddu s3, zero, zero, //Counter
daddu v0, zero, zero

_FFRReLoop:
daddu s4, s0, zero //Place holder for return

_FFRLoop:
slt t0, s0, s1
beq t0, zero, :_FFREnd
nop

lq t0, $0000(s0)

bne t0, zero, :_FFRNotZ
nop

slt t0, s3, s2
beq t0, zero, :_FFRFoundR
nop

addiu s3, s3, $0010
beq zero, zero, :_FFRLoop
addiu s0, s0, $0010

_FFRNotZ:
addiu s0, s4, $0100 //Skip 0x800 addresses (for sake of speed)
addu s0, s0, s3
daddu s3, zero, zero

beq zero, zero, :_FFRReLoop
nop

_FFRFoundR:
daddu v0, s4, zero

_FFREnd:
lq s0, $0000(sp)
lq s1, $0010(sp)
lq s2, $0020(sp)
lq s3, $0030(sp)
lq s4, $0040(sp)
jr ra
addiu sp, sp, $0050
*/

//==================================================
/*
Parse joker type
Takes a jokers type and joker address and returns 1 if true
Arguments:
a0 = address
a1 = conditional value
a2 = type
a3 = width (byte, half, word: 0, 1, 2)
Return:
v0 = 1 if true, 0 if false

From CMP.net
*
0 - Equal-to (value = xxxx).
*
1 - Not equal-to (value != xxxx).
*
2 - Less than or equal (value ? xxxx).
*
3 - Greater than or equal (value >= xxxx).
*
4 - Mask unset (value AND xxxx = 0). Test to see if particular bits are unset (all bits must be unset). Useful for joker codes.
*
5 - Mask set (value AND xxxx != 0). Test to see if particular bits are set (any of the bits). Useful for joker codes.
*
6 - Similar to 5, except all the bits in the mask must be set for the next codes to execute ([value AND xxxx] - xxxx = 0).
*/
_ParseJokerType:
addiu sp, sp, $FFC0
sq s0, $0000(sp)
sq s1, $0010(sp)
sq s2, $0020(sp)
sq s3, $0030(sp)

daddu s0, a0, zero
daddu s1, a1, zero
daddu s2, a2, zero
daddu s3, a3, zero
daddu v0, zero, zero

bne s3, zero, 2
nop
lhu t0, $0000(s0) //Load half from address

addiu v1, zero, 1
bne s3, v1, 2
nop
lb t0, $0000(s0) //Load byte from address

addiu v1, zero, 2
bne s3, v1, 2
nop
lw t0, $0000(s0) //Load word from address

beq s2, zero, :_PJT_0
nop

addiu v1, zero, 1
beq s2, v1, :_PJT_1
nop

addiu v1, zero, 2
beq s2, v1, :_PJT_2
nop

addiu v1, zero, 3
beq s2, v1, :_PJT_3
nop

addiu v1, zero, 4
beq s2, v1, :_PJT_4
nop

addiu v1, zero, 5
beq s2, v1, :_PJT_5
nop

addiu v1, zero, 6
beq s2, v1, :_PJT_6
nop

beq zero, zero, :_PJT_Exit
nop

_PJT_0: //Equal to
bne t0, s1, :_PJT_Exit
nop

addiu v0, zero, 1

beq zero, zero, :_PJT_Exit
nop

_PJT_1: //Not equal to
beq t0, s1, :_PJT_Exit
nop

addiu v0, zero, 1

beq zero, zero, :_PJT_Exit
nop

_PJT_2: //Less than or equal
slt t0, t0, s1 //This works as a <= instead of using two slt's

bne t0, zero, :_PJT_Exit
nop

addiu v0, zero, 1

beq zero, zero, :_PJT_Exit
nop

_PJT_3: //Greater than or equal
slt t0, s1, t0 //This works as a <= instead of using two slt's

bne t0, zero, :_PJT_Exit
nop

addiu v0, zero, 1

beq zero, zero, :_PJT_Exit
nop

_PJT_4: //Mask unset (ADDR & VALUE == 0)
sll s1, s1, 16
srl s1, s1, 16
and t1, s1, t0

bne t1, zero, :_PJT_Exit
nop

addiu v0, zero, 1

beq zero, zero, :_PJT_Exit
nop

_PJT_5: //Mask set (ADDR & VALUE != 0)
sll s1, s1, 16
srl s1, s1, 16
and t1, s1, t0

beq t1, zero, :_PJT_Exit
nop

addiu v0, zero, 1

beq zero, zero, :_PJT_Exit
nop

_PJT_6: //All but value ((VALUE & ADDR) - VALUE == 0)
sll s1, s1, 16
srl s1, s1, 16
and t1, s1, t0 //Just going with what CMP said... Never used this
subu t1, t1, t0

bne t1, zero, :_PJT_Exit
nop

addiu v0, zero, 1

beq zero, zero, :_PJT_Exit
nop

_PJT_Exit:
lq s0, $0000(sp)
lq s1, $0010(sp)
lq s2, $0020(sp)
lq s3, $0030(sp)
jr ra
addiu sp, sp, $0040

//==========================================================
/*
a0 = value
a1 = small between
a2 = big between
v0 returns 0 if it is between, 1 if it is not
*/
_IfBetween:
slt v0, a0, a1
slt v1, a2, a0
jr ra
or v0, v0, v1 //If v0 or v1 is 1, returns 1. Otherwise returns 0

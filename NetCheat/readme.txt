NetCheat consists of NetCheat.elf, NetCheatManager.exe, cb2util (made by misfire), and zlib.dll
Developed by Dnawrkshp and ORCXodus
Product of hardwork and http://www.cod-orc.com/customwarez.htm

Source code can be grabbed at my GitHub: www.GitHub.com/Dnawrkshp

Maximum code limits:
OpenPS2Loader: 		8192 lines of code
ESR:				Unknown (probably the same as a disc)
Disc:				10752 lines of code
DEBUG mode:			2832 lines of code

IP Settings:
NetCheat uses the same file that uLaunchELF uses for your network config.
That file should be at mc?:/SYS-CONF/IPCONFIG.DAT, where ? is either a 0 or a 1.
If you have uLaunchELF, then you can go to the settings menu (SELECT) and going to Network Settings.
Then you will be able to change your IP address, netmask, and gateway.

Labels:
Supports standard labels:
Rapid fire
20347E8C 00000000

NetCheat codetypes:
For all the codetypes not explained, please refer to this: http://www.codemasters-project.net/guides/showentry.php?e=847
The ones below either are new or have something different.
My thanks to Pyriel for making such a thorough guide on the codetypes of CodeBreaker.

------------------------- Code Type 4
Multi-address Write (slide-fill code)
4aaaaaaa wcccssss
xxxxxxxx iiiiiiii

a: 25-bit address.
w: Width (0 = 32 bit, 1 = 16 bit, 2 = 8 bit)
c: Number of times to write.
s: Step value (w = 0: s = (s * 4), w = 1: s = (s * 2), w = 2: s = s.
x: 32-bit value.
i: 32-bit value increment.

By Pyriel:
The 32-bit value xxxxxxxx will be stored at the address given by aaaaaaa.
The count ccc will be decremented.
The step value ssss will be multiplied by (w = 0:4; w = 1:2; w = 2:1) and added to the address.
The value xxxxxxxx will have iiiiiiii added to it.
This process will continue until cccc reaches zero.
Basically, write to ccc addresses, while jumping ssss * (4 - (w * 2)) addresses in between.

Example:
40EE7174 00060002
00000063 00000001
------------------------- Code Type 9
Conditional mastercode
9aaaaaaa vvvvvvvv
a: Address
v: 32-bit value

NetCheat will read the value at aaaaaaa and compare it with vvvvvvvv.
If they are equal, NetCheat will execute it's engine.
Otherwise it will not and no cheats will run.
If you do not include a 9 type mastercode, NetCheat will run the moment it is hooked and a syscall is called

Example:
9013BA48 00832021
------------------------- Code Type A
Kernel write
Aaaaaaaa vvvvvvvv
a: Address
v: 32-bit value

The value v will be stored at 0x8aaaaaaa (KSEG0)

Example:
A0070000 1337D00D
------------------------- Code Type B
Delay
B0000000 0vvvvvvv

v: 28-bit value

The engine will execute all the codes above vvvvvvv times before executing the one below

Example:
B0000000 00000100
------------------------- Code Type F1
Execute Data / Hook - Idea by Gtlcpimp
F10000dd 0aaaaaaa

a: Address
d: 8-bit value

The engine will run dd times before executing a jalr to aaaaaaa

Example:
F100000E 000C0000

------------------------- Code Type F2
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

-------------------------
The rest can once again be accessed at: http://www.codemasters-project.net/guides/showentry.php?e=847
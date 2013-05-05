ELF Loader by Dnawrkshp and Berion

This was originally taken from Berion's Artemis GUI, then put into working order by Dnawrkshp in the Unofficial CL-LiveDebug v4.
Gets loaded into the memory and executed with the bootpath as argv[0].
It then boots bootpath using SifLoadElf. If that fails it manually loads the elf and executes it with ExecPS2.

DEBUG COLORS:
Orange	=	SifLoadElf
Yellow	=	ExecPS2
Red		=	Opening ELF
Green	=	Reading ELF
Blue	=	Loading ELF into RAM
Pink	=	Launching OSDSYS
White	=	Failed to launch OSDSYS

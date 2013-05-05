/* From CL-LiveDebug v4 */

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <string.h>
#include <stdio.h>

/* GS macro */
#define GS_BGCOLOUR	*((vu32*)0x120000e0)

char boot_path[1024];
int load_elf_ram(char *elf);

/* Debug colors. Now NTSC Safe! At least I believe they are... */
int red = 0x1010B4; /* RED: Opening elf */
int green = 0x10B410; /* GREEN: Reading elf */
int blue = 0xB41010; /* BLUE: Installing elf header */
int yellow = 0x10B4B4; /* YELLOW: ExecPS2 */
int orange = 0x0049E6; /* ORANGE: SifLoadElf */
int pink = 0xB410B4; /* PINK: Launching OSDSYS */
int white = 0xEBEBEB; /* WHITE: Failed to launch OSDSYS */
int purple = 0x801080; /* PURPLE: Launching uLaunchELF */

/* ELF-header structures and identifiers */
#define ELF_MAGIC	0x464c457f
#define ELF_PT_LOAD	1

typedef struct {
	u8	ident[16];
	u16	type;
	u16	machine;
	u32	version;
	u32	entry;
	u32	phoff;
	u32	shoff;
	u32	flags;
	u16	ehsize;
	u16	phentsize;
	u16	phnum;
	u16	shentsize;
	u16	shnum;
	u16	shstrndx;
} elf_header_t;

typedef struct {
	u32	type;
	u32	offset;
	void *vaddr;
	u32	paddr;
	u32	filesz;
	u32	memsz;
	u32	flags;
	u32	align;
} elf_pheader_t;

/*
 * launch OSDSYS in case elf load failed (if fmcb installed, reloads it)
 */
void launch_OSDSYS(void)
{
	__asm__ __volatile__(
		"	li $3, 0x04;"
		"	syscall;"
		"	nop;"
	);
}

/*
 * main function
 */
int main (int argc, char *argv[1])
{
	int i;
	
	/* preserves elf path and ule path before clearing stack !!! */
	strcpy(boot_path, argv[0]);
	
	//Taken from Open PS2 Loader's elfldr.c
	ResetEE(0x7f);
	
	/* Clear user memory */
	for (i = 0x00100000; i < 0x02000000; i += 64) {
		__asm__ (
			"\tsq $0, 0(%0) \n"
			"\tsq $0, 16(%0) \n"
			"\tsq $0, 32(%0) \n"
			"\tsq $0, 48(%0) \n"
			:: "r" (i)
		);
	}

	/* Clear scratchpad memory */
	memset((void*)0x70000000, 0, 16 * 1024);
	
	load_elf_ram(boot_path);

	//If that fails, then boot OSDSYS
	GS_BGCOLOUR = pink; /* PINK: Launching OSDSYS */
  	SifLoadFileExit();
  	fioExit();
  	SifExitRpc();
  	launch_OSDSYS();

 	GS_BGCOLOUR = white; /* WHITE: Failed to load OSDSYS */
  	SleepThread();
	//execute_elf(argv[0]);

	return -1;
}

int load_elf_ram(char *elf) {
	char *args[1];
	t_ExecData sifelf;
	int r;
	
	memset(&sifelf, 0, sizeof(t_ExecData));
    SifLoadFileInit();
	GS_BGCOLOUR = orange; /* ORANGE: SifLoadElf */
	r = SifLoadElf(elf, &sifelf);
	
	if ((!r) && (sifelf.epc)) {
		
		GS_BGCOLOUR = yellow; /* YELLOW: ExecPS2 */
  	    SifLoadFileExit();
  	  	fioExit();
  	  	SifExitRpc(); // some programs need it to be here

  	  	FlushCache(0);
  	  	FlushCache(2);
		
    	args[0] = elf;
    	ExecPS2((void*)sifelf.epc, (void*)sifelf.gp, 1, args);
  	} else {
	
		u32 elfloc = 0x1800000;
		elf_header_t *boot_header;
		elf_pheader_t *boot_pheader;
		int i, size = 0, fd = 0;
	
		GS_BGCOLOUR = red; /* RED: Opening elf */
		fd = fioOpen(elf, O_RDONLY); //Open the elf
		if (fd < 0) { fioClose(fd); return -1; } //If it doesn't exist, return

		size = fioLseek(fd, 0, SEEK_END); //Gets the size of the elf
		fioLseek(fd, 0, SEEK_SET);
	
		if (size < 0) { fioClose(fd); return -2; }
	
		GS_BGCOLOUR = green; /* GREEN: Reading elf */
		fioRead(fd, (u32*)elfloc, size); //Reads the elf and stores it into the memory at 0x01800000
		fioClose(fd); //Closes system.cnf
		elfloc = 0x1800000;
	
		/* Get Elf header */
		GS_BGCOLOUR = blue; /* BLUE: Installing elf header */
		boot_header = (elf_header_t*)elfloc;
	
		/* Check elf magic */
		if ((*(u32*)boot_header->ident) != ELF_MAGIC)
			return -2;

		/* Get program headers */
		boot_pheader = (elf_pheader_t*)(elfloc + boot_header->phoff);

		/* Scan through the ELF's program headers and copy them into apropriate RAM
		* section, then padd with zeros if needed.
		*/
		for (i = 0; i < boot_header->phnum; i++) {
			if (boot_pheader[i].type != ELF_PT_LOAD)
				continue;

			memcpy(boot_pheader[i].vaddr, (u8 *)elfloc + boot_pheader[i].offset, (int)(boot_pheader[i].filesz)); //(elf_pheader_t*)boot_pheader[i].filesz);

			if (boot_pheader[i].memsz > boot_pheader[i].filesz)
				memset(boot_pheader[i].vaddr + boot_pheader[i].filesz, 0, boot_pheader[i].memsz - boot_pheader[i].filesz);
		}
	
		if (!boot_header->entry) //If the entrypoint is 0 exit
			return -3;
	
		//Booting part
		GS_BGCOLOUR = yellow; /* YELLOW: ExecPS2 */
		SifLoadFileExit();
		fioExit();
		SifExitRpc(); /* Some programs need it to be here */

		FlushCache(0);
		FlushCache(2);
		
		args[0] = elf;
		ExecPS2((u32*)boot_header->entry, 0, 1, args);
	}
	return -1;
}

#include <stdio.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <debug.h>
#include <tamtypes.h>
#include <loadfile.h>
#include <fileio.h>
#include <ps2ip.h>
#include "smap/smap.h"
#include <sbv_patches.h>
#include <gsKit.h>

#include "src/qwerty.c" //CogSwap Source - I copied from Gtlcpimp's LDv3

//#define DEBUG

/* CL-LiveDebug */
#ifdef DEBUG
#include "Exports/LiveDebug.c"
#endif

/* Engine */
#include "Exports/NCEngine.c"

/* Kernel/Syscall hook address and value */
#ifdef DEBUG
u32 HookAddr = 0x800002FC;
u32 HookValue = (0x00047000 / 4) + 0x0C000000;
#else
u32 HookAddr = 0x800001A0;
u32 HookValue = (0x00047000 / 4) + 0x08000000;
#endif

u32 EngineAddr = 0x80047000;
u32 CodesAddr = 0x80079000;

extern void usbd_irx;
extern void usb_mass_irx;
extern void elf_loader;
extern void dns_irx;
extern void poweroff_irx;
extern void ps2dev9_irx;
extern void ps2ip_irx;
extern void ps2ips_irx;
extern void smap_irx;

extern u32 size_usbd_irx;
extern u32 size_usb_mass_irx;
extern u32 size_elf_loader;
extern u32 size_dns_irx;
extern u32 size_poweroff_irx;
extern u32 size_ps2dev9_irx;
extern u32 size_ps2ip_irx;
extern u32 size_ps2ips_irx;
extern u32 size_smap_irx;

/* main.c */
char *ReadIP();
char *ReadNM();
char *ReadGW();
void ExecGame(int op, char *elf, u32 CheatLoc);
int LoadIRX();
int StartServer();
char *ReadBoot();
int ReadMaster();
int ParseMastercode();
void StopDisc(void);
void load_elf(char *elf_path, int code_len);
void spinCD(void);
void IOP_Reset(void);
void ResetCNF(void);
void CleanUp(void);
int _recv(int s, char *buf, int len, int flags, int mode);
int _send(int s, char *buf, int len, int flags, int mode);
void WaitForOkay(int s);
int ReplyWithOkay(int s);
void UpdateMMenu(char *text);
int LoadSettings(void);
int LoadModules(void);
void delay(int count);
int ReplyWithError(int s, char *str);

/* From gui.c */
extern void Setup_GS(int gs_vmode);
extern void gfx_set_defaults(void);
extern void vram_free(void);
extern void load_mainmenu_Textures(void);
extern void load_Font(void);
extern void Clear_Screen(void);
extern void Render_GUI(void);
extern void load_background_Textures(void);
extern void AnimateFade(int start, int cnd, int rate, int delay);
extern void load_Font(void);
extern int Draw_MainMenu(void);
extern int Draw_WaitMenu(int done, int max);

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int SCREEN_X;
extern int SCREEN_Y;
extern float Y_RATIO;
extern int TV_mode;
extern char *midString[1024];
extern char *ipString[13];
extern int background_alpha;
extern int logo_alpha;

/* timer.c */
extern void TimerInit(void);
extern u64  Timer(void);
extern void TimerEnd(void);

/* Global Declarations */
int z = 0;											/* Server connection status */
char *sys_cnf = "cdrom0:\\SYSTEM.CNF;1";			/* SYSTEM.CNF location */
int wLoad = 0;										/* Number of modules loaded */
#ifdef DEBUG
int wMax = 11;										/* Number of modules to load */
#else
int wMax = 14;										/* Number of modules to load */
#endif

/* Recieving values */
#define 	ST_GM	0x1		/* Start game */
#define 	D_CON	0x2		/* Disconnect */
#define 	RCV_C	0x3		/* Receive codes */
#define 	ALT_B	0x4		/* Alternative boot */
#define 	STP_D	0x5		/* Stop disc */

/* Error values */


/* Settings */
char ip[13];										/* IP Address */
char nm[13];										/* Netmask */
char gw[13];										/* Gateway */
char run_path[1024];								/* Directory of NetCheat.elf passed through argv[0] */

/* GUI */
u8  romver[16];
char *midS[1024];

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

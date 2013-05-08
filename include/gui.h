#include <tamtypes.h>
#include <kernel.h>
#include <gsKit.h>
#include <dmaKit.h>
#include <loadfile.h>
#include <png.h>
#include <string.h>
#include <stdio.h>

/* Regardless of this, nothing seems to output (ps2ip_init()'s fault). Best to leave it commented */
//#define DEBUG

extern void load_elf(char *elf_path);

extern void background;
extern void logo;
extern void font_bitsumishi;

extern u32 size_background;
extern u32 size_logo;
extern u32 size_font_bitsumishi;

/* include font specific datas */
#include "include/font_bitsumishi.h"

struct about_content {
	char *name;
	char *desc;
};

/* fn prototypes */
void Setup_GS(int gs_vmode);
void gfx_set_defaults(void);
void vram_free(void);
void load_Font(void);
int Draw_MainMenu(void);
void Clear_Screen(void);
void Render_GUI(void);
void draw_background(int alpha);
void load_background_Textures(void);
void AnimateFade(int start, int cnd, int rate, int delay);
int Draw_WaitMenu(int done, int max);

GSGLOBAL *gsGlobal;
GSTEXTURE tex_background;		//Background
GSTEXTURE tex_logo;				//Logo
GSTEXTURE tex_font_bitsumishi;	//Font

/* screen defaults for NTSC, just in case */
int TV_mode       = 2;
int	SCREEN_WIDTH  = 640;
int	SCREEN_HEIGHT = 448;
int SCREEN_X	  = 632;
int SCREEN_Y	  = 50;
float Y_RATIO	  = 0.875f;

/* define colors */
#define Black  		GS_SETREG_RGBAQ(0x00,0x00,0x00,0x80,0x00)
#define White		GS_SETREG_RGBAQ(0xFF,0xFF,0xFF,0x80,0x00)

/* For GUI */
int background_alpha;
int logo_alpha;
char *midString[1024];
char *ipString[13];
char *waitString[100];

/*
 * PNG handling code: from MyPS2 by ntba2
 */
typedef struct {
	int width;
	int height;
	int bit_depth;
	void *priv;
} pngData;

typedef struct {
	png_structp	png_ptr;
	png_infop	info_ptr, end_info;
	u8 *buf;
	int pos;
	u8 *data;
} pngPrivate;

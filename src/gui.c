#include "include/gui.h"

/*
 * read_data_fn
 */
static void read_data_fn(png_structp png_ptr, png_bytep buf, png_size_t size) {
	pngPrivate *priv = (pngPrivate*)png_get_io_ptr(png_ptr);

	memcpy(buf, priv->buf + priv->pos, size);
	priv->pos += size;
}

/*
 * pngOpenRAW
 */
pngData *pngOpenRAW(u8 *data, int size) {
	pngData		*png;
	pngPrivate	*priv;

	if (png_sig_cmp( data, 0, 8 ) != 0)
		return NULL;
		
	if ((png = malloc(sizeof(pngData))) == NULL)
		return NULL;

	memset (png, 0, sizeof(pngData));

	if ((priv = malloc(sizeof(pngPrivate))) == NULL)
		return NULL;

	png->priv = priv;

	priv->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!priv->png_ptr) {
		#ifdef DEBUG
			printf("PNG Read Struct Init Failed\n");
		#endif
		free(png);
		return NULL;
	}

	priv->info_ptr = png_create_info_struct(priv->png_ptr);
	if (!priv->info_ptr) {
		#ifdef DEBUG
			printf("PNG Info Struct Init Failed\n");
		#endif
		free(png);
		png_destroy_read_struct(&priv->png_ptr, NULL, NULL);
		return NULL;
	}

	priv->end_info = png_create_info_struct(priv->png_ptr);
	if (!priv->end_info) {
		free(png);
		png_destroy_read_struct(&priv->png_ptr, &priv->info_ptr, NULL);
		return NULL;
	}
	
	priv->buf	= data;
	priv->pos	= 0;

	png_set_read_fn(priv->png_ptr, (png_voidp)priv, read_data_fn);
	png_read_png(priv->png_ptr, priv->info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	png->width	= priv->info_ptr->width;
	png->height	= priv->info_ptr->height;
	png->bit_depth	= priv->info_ptr->channels * 8;

	#ifdef DEBUG
		printf("PNG info: width=%3d ", png->width);
		printf("height=%3d ", png->height);	
		printf("height=%3d ", png->height);	
		printf("bit depth=%2d ", png->bit_depth);
		printf("color type=%2d\n", priv->png_ptr->color_type);
	#endif
		
	return png;
}

/*
 * pngReadImage
 */
int pngReadImage(pngData *png, u8 *dest) {
	pngPrivate *priv = png->priv;
	u8 **row_pointers;
	int i, row_ptr;

	int y;

	row_pointers = png_get_rows(priv->png_ptr, priv->info_ptr);
	row_ptr = 0;		

	for (i = 0; i < priv->info_ptr->height; i++) {
		memcpy(dest + row_ptr, row_pointers[i], priv->info_ptr->rowbytes);

		/* need to normalize alpha channel to ps2 range */
		if (priv->info_ptr->channels == 4) {
			for (y = 3; y < priv->info_ptr->rowbytes; y += 4)
				*(dest + row_ptr + y ) /= 2;
		}

		row_ptr += priv->info_ptr->rowbytes;
	}

	return 1;
}

/*
 * pngClose
 */
void pngClose(pngData *png) {
	pngPrivate *priv = png->priv;

	png_destroy_read_struct(&priv->png_ptr, &priv->info_ptr, &priv->end_info);
	
	if (priv->data)
		free(priv->data);

	free(priv);
	free(png);
}

/*
 * Draw a character with bitsumishi font
 */
void drawChar_bitsumishi(float x, float y, int alpha, u32 width, u32 height, u32 c) {
	float x1, x2, y1, y2;
	int u1, u2, v1, v2;

	x1 = x;
	x2 = x1 + width;
	y1 = y;
	y2 = y1 + height;

	c -= 32;
	u1 = (c % (tex_font_bitsumishi.Width/32)) * (tex_font_bitsumishi.Width/16);
	u2 = u1 + 32;
	v1 = (c - (c % 16)) * 2; /* careful: 6 rows only !!! */
	v2 = v1 + 32;

	/* Draw a char using neuropol texture */
	gsKit_prim_sprite_texture(gsGlobal, &tex_font_bitsumishi,
							x1, /* X1 */
							y1,	/* Y1 */
							u1, /* U1 */
							v1, /* V1 */
							x2, /* X2 */
							y2,	/* Y2 */
							u2, /* U2 */
							v2, /* V2 */
							0,
							White);
}

/*
 * Draw a string with bitsumishi font
 */
void drawString_bitsumishi(u32 x, u32 y, int alpha, int fontsize, int fontspacing, const char *string) {
	int l, i;
	float cx;
	int c;

	cx = x;

	l = strlen(string);

	for( i = 0; i < l; i++)	{
		c = (u8)string[i];

		/* catch "\n" */
		if (c == 10) {
			y += fontsize * Y_RATIO;
			cx = x;
		}
		
		/* Draw the string character by character */
		drawChar_bitsumishi(cx, y, alpha, fontsize, fontsize * Y_RATIO, c);

		/* Uses width informations for neuropol font header file */
		if (c != 10) {
			float f = font_bitsumishi_width[c-32] * (float)(fontsize / 32.0f);
			cx += (float)(f + fontspacing);
		}
	}
}

/*
 * Calculate and return width in pixels of a string using bitsumishi font
 */
int getStringWidth_bitsumishi(const char *string, int fontsize, int fontspacing) {
	int i, l, c;
	float size;

	l = strlen(string);

	size = 0;

	for( i = 0; i < l; i++) {
		c = (u8)string[i];
		
		float f = font_bitsumishi_width[c-32] * (float)(fontsize / 32.0f);
		size += (float)(f + fontspacing);
	}

	return (int)size;
}

/*
 * Calculate and return height in pixels of a string using bitsumishi font
 */
int getStringHeight_bitsumishi(const char *string, int fontsize) {
	int l, i, c;
	int height = fontsize * Y_RATIO;

	l = strlen(string);

	for( i = 0; i < l; i++)	{
		c = (u8)string[i];
		if (c > 127) c = 127; /* security check as the font is incomplete */

		/* catch "\n" */
		if (c == 10)
			height += fontsize * Y_RATIO;		
	}
	
	return height;
}

/*
 * Draw a centered string with bitsumishi font
 */
void centerString_bitsumishi(int y, int alpha, int fontsize, int fontspacing, const char *string) {
	int x, str_width;
	
	str_width = getStringWidth_bitsumishi(string, fontsize, fontspacing);
	
	x = (SCREEN_WIDTH - str_width) / 2;
	
	drawString_bitsumishi(x, y, alpha, fontsize, fontspacing, string);
}

/*
 * Draw background texture
 */
void draw_background(int alpha) {
	
	gsKit_prim_sprite_texture(gsGlobal, &tex_background,
							0, 						/* X1 */
							0,  					/* Y1 */
							0,  					/* U1 */
							0,  					/* V1 */
							SCREEN_WIDTH,			/* X2 */
							SCREEN_HEIGHT,			/* Y2 */
							tex_background.Width, 	/* U2 */
							tex_background.Height, 	/* V2 */
							0,
							GS_SETREG_RGBAQ(0x80, 0x80, 0x80, alpha, 0x00));
}

/*
 * Draw NetCheat logo / credits text
 */
void draw_logo(int alpha) {
	int x, y;

	x = (SCREEN_WIDTH - tex_logo.Width) / 2;
	y = (18 * Y_RATIO) + 50;

	/* Draw logo */
	gsKit_prim_sprite_texture(gsGlobal, &tex_logo,
							x, 								/* X1 */
							y,								/* Y1 */
							0,  							/* U1 */
							0,  							/* V1 */
							x + tex_logo.Width, 			/* X2 */
							y + (tex_logo.Height * Y_RATIO),/* Y2 */
							tex_logo.Width, 				/* U2 */
							tex_logo.Height,				/* V2 */
							0,
							GS_SETREG_RGBAQ(0x80, 0x80, 0x80, alpha, 0x00));
}

/*
 * Draws the texture tex
 */
void draw_image(GSTEXTURE tex, int x, int y, int alpha) {
	gsKit_prim_sprite_texture(gsGlobal, &tex,
							x, 										/* X1 */
							y,										/* Y1 */
							0,  									/* U1 */
							0,  									/* V1 */
							x + tex.Width, 							/* X2 */
							y + (tex.Height * Y_RATIO), 			/* Y2 */
							tex.Width, 								/* U2 */
							tex.Height,								/* V2 */
							0,
							GS_SETREG_RGBAQ(0x80, 0x80, 0x80, alpha, 0x00));
}

/*
 * Clears VRAM
 */
void vram_free(void) {
	gsKit_vram_clear(gsGlobal);
}

/*
 * Load mainmenu textures into VRAM
 */
void load_background_Textures(void) {
	pngData *pPng;
	u8		*pImgData;
	
	#ifdef DEBUG
		printf("1st VRAM Pointer = %08x  \n", gsGlobal->CurrentPointer);
	#endif
	
	/* gsGlobal->CurrentPointer = vram_pointer; */
	
	if ((pPng = pngOpenRAW(&background, size_background)) > 0) { /* tex size = 0x140000 */
		if ((pImgData = malloc(pPng->width * pPng->height * (pPng->bit_depth / 8))) > 0) {
			if (pngReadImage( pPng, pImgData ) != -1) {
				tex_background.PSM 		= GS_PSM_CT32;
				tex_background.Mem 		= (u32 *)pImgData;
				tex_background.VramClut = 0;
				tex_background.Clut		= NULL;
				tex_background.Width    = pPng->width;
				tex_background.Height   = pPng->height;
				tex_background.Filter   = GS_FILTER_LINEAR;
				#ifdef DEBUG
					printf("VRAM Pointer = %08x  ", gsGlobal->CurrentPointer);
					printf("texture size = %x\n", gsKit_texture_size(pPng->width, pPng->height, GS_PSM_CT32));
				#endif
				tex_background.Vram 	= gsKit_vram_alloc(gsGlobal,
			 						  		gsKit_texture_size(tex_background.Width, tex_background.Height, tex_background.PSM),
			 						  		GSKIT_ALLOC_USERBUFFER);
				gsKit_texture_upload(gsGlobal, &tex_background);
			}
			pngClose(pPng);
			free(pImgData);
		}
	}

	if ((pPng = pngOpenRAW(&logo, size_logo)) > 0) {
		if ((pImgData = malloc(pPng->width * pPng->height * (pPng->bit_depth / 8))) > 0) {
			if (pngReadImage( pPng, pImgData ) != -1) {
				tex_logo.PSM 	  = GS_PSM_CT32;
				tex_logo.Mem 	  = (u32 *)pImgData;
				tex_logo.VramClut = 0;
				tex_logo.Clut	  = NULL;
				tex_logo.Width    = pPng->width;
				tex_logo.Height   = pPng->height;
				tex_logo.Filter   = GS_FILTER_LINEAR;
				#ifdef DEBUG
					printf("VRAM Pointer = %08x  ", gsGlobal->CurrentPointer);
					printf("texture size = %x\n", gsKit_texture_size(pPng->width, pPng->height, GS_PSM_CT32));
				#endif
				tex_logo.Vram 	  = gsKit_vram_alloc(gsGlobal,
	 						  			gsKit_texture_size(tex_logo.Width, tex_logo.Height, tex_logo.PSM),
	 						  			GSKIT_ALLOC_USERBUFFER);
				gsKit_texture_upload(gsGlobal, &tex_logo);
			}
			pngClose(pPng);
			free(pImgData);
		}
	}
	
	#ifdef DEBUG
		printf("Load_GUI last VRAM Pointer = %08x  \n", gsGlobal->CurrentPointer);
	#endif
	//SleepThread();		
}

/*
 * Load the font into VRAM
 */
void load_Font(void) {
	pngData *pPng;
	u8		*pImgData;
	
	#ifdef DEBUG
		printf("1st VRAM Pointer = %08x  \n", gsGlobal->CurrentPointer);
	#endif

	/* gsGlobal->CurrentPointer = vram_pointer; */

	if ((pPng = pngOpenRAW(&font_bitsumishi, size_font_bitsumishi)) > 0) { /* tex size = 0x140000 */
		if ((pImgData = malloc(pPng->width * pPng->height * (pPng->bit_depth / 8))) > 0) {
			if (pngReadImage( pPng, pImgData ) != -1) {
				tex_font_bitsumishi.PSM 		= GS_PSM_CT32;
				tex_font_bitsumishi.Mem 		= (u32 *)pImgData;
				tex_font_bitsumishi.VramClut = 0;
				tex_font_bitsumishi.Clut		= NULL;
				tex_font_bitsumishi.Width    = pPng->width;
				tex_font_bitsumishi.Height   = pPng->height;
				tex_font_bitsumishi.Filter   = GS_FILTER_LINEAR;
				#ifdef DEBUG
					printf("VRAM Pointer = %08x  ", gsGlobal->CurrentPointer);
					printf("texture size = %x\n", gsKit_texture_size(pPng->width, pPng->height, GS_PSM_CT32));
				#endif
				tex_font_bitsumishi.Vram 	= gsKit_vram_alloc(gsGlobal,
			 						  		gsKit_texture_size(tex_font_bitsumishi.Width, tex_font_bitsumishi.Height, tex_font_bitsumishi.PSM),
			 						  		GSKIT_ALLOC_USERBUFFER);
				gsKit_texture_upload(gsGlobal, &tex_font_bitsumishi);
			}
			pngClose(pPng);
			free(pImgData);
		}
	}
	#ifdef DEBUG
		printf("Load_GUI last VRAM Pointer = %08x  \n", gsGlobal->CurrentPointer);
	#endif
	//SleepThread();
}

/*
 * Careful !!! Must be called after Load_Texture function, and before drawing anything !
 * Init default for many gfx functions
 */
void gfx_set_defaults(void)
{
	/* Init defaults for GUI */
	logo_alpha = 128;
	background_alpha = 128;
}

/*
 * setup GS
 */
void Setup_GS(int gs_vmode)
{
	/* GS Init */
	gsGlobal = gsKit_init_global_custom(
		GS_RENDER_QUEUE_OS_POOLSIZE, //+GS_RENDER_QUEUE_OS_POOLSIZE/2, /* eliminates overflow */
		GS_RENDER_QUEUE_PER_POOLSIZE);

	/* Clear Screen */
	gsKit_clear(gsGlobal, Black);

	/* Mode & screen width/height init */
	gsGlobal->Mode = gs_vmode;
    gsGlobal->Width = SCREEN_WIDTH;
    gsGlobal->Height = SCREEN_HEIGHT;

	/* Screen Position Init */
	gsGlobal->StartX = SCREEN_X;
	gsGlobal->StartY = SCREEN_Y;

	/* Buffer Init */
	gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
	gsGlobal->PrimAAEnable = GS_SETTING_ON;
	gsGlobal->DoubleBuffering = GS_SETTING_OFF;
	gsGlobal->ZBuffering      = GS_SETTING_OFF;
	gsGlobal->PSM = GS_PSM_CT32;
	
	/* Force Interlace and Field mode */
	gsGlobal->Interlace = GS_INTERLACED;
	gsGlobal->Field     = GS_FIELD;

	/* DMAC Init */
	dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_FROMSPR);
	dmaKit_chan_init(DMA_CHANNEL_TOSPR);

	/* Screen Init */
	gsKit_init_screen(gsGlobal);
	gsKit_clear(gsGlobal, Black);
		
	gsKit_mode_switch(gsGlobal, GS_PERSISTENT);
}

/*
 * render GUI
 */
void Render_GUI(void)
{				     
	/* Flips Framebuffers on VSync */
	gsKit_sync_flip(gsGlobal);

	/* Normal User Draw Queue "Execution" (Kicks Oneshot and Persistent Queues) */
	gsKit_queue_exec(gsGlobal);	
		
	gsKit_queue_reset(gsGlobal->Per_Queue);
}

/*
 * clear screen
 */
void Clear_Screen(void)
{
	/* Clear screen	*/
	gsKit_clear(gsGlobal, Black);
}

/*
 * Animates fading of the background
 */
void AnimateFade(int start, int end, int rate, int delay) {
	int x = 0;
	
	if (rate > 0) {
		for (background_alpha = start; background_alpha < end; background_alpha += rate) {
			for (x = 0; x < delay; x++) { ;; } //Delay
		
			Clear_Screen();
			gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
			gsKit_set_test(gsGlobal, GS_ATEST_OFF);
			draw_background(background_alpha);
			Render_GUI();
		}
	} else if (rate < 0) {
		for (background_alpha = start; background_alpha > end; background_alpha += rate) {
			for (x = 0; x < delay; x++) { ;; } //Delay
			
			Clear_Screen();
			gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
			gsKit_set_test(gsGlobal, GS_ATEST_OFF);
			draw_background(background_alpha);
			Render_GUI();
		}
	}
	
	Render_GUI();
}
 
/*
 * Draw MainMenu
 */
int Draw_MainMenu(void) {
	/* Clear screen	*/
	gsKit_clear(gsGlobal, Black);

	/* Set Alpha settings */
	gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
	gsKit_set_test(gsGlobal, GS_ATEST_OFF);

	/* Draw Background */
	draw_background(background_alpha);
				
	/* Draw logo */
	draw_logo(logo_alpha);
	
	centerString_bitsumishi(((SCREEN_HEIGHT/2) + 120) * Y_RATIO, 128, 30, 0, (char*)ipString);
	centerString_bitsumishi(((SCREEN_HEIGHT/2) + 160) * Y_RATIO, 128, 25, 0, (char*)midString);
	
    gsKit_set_test(gsGlobal, GS_ATEST_ON);
    
    /* Blend Alpha Primitives "Back To Front" */
    gsKit_set_primalpha(gsGlobal, GS_BLEND_BACK2FRONT, 0);
    
    return 1;
}

/*
 * Draw WaitMenu
 */
int Draw_WaitMenu(int done, int max) {
	
	Clear_Screen();
	gsKit_set_primalpha(gsGlobal, GS_SETREG_ALPHA(0,1,0,1,0), 0);
	gsKit_set_test(gsGlobal, GS_ATEST_OFF);
	
	sprintf((char*)waitString, "Loading modules: %2.0f%c", ((float)done / (float)max) * 100, (char)'%');
	centerString_bitsumishi(((SCREEN_HEIGHT/2) + 120) * Y_RATIO, 128, 30, 0, (char*)waitString);
	
	Render_GUI();
    
    return 1;
}

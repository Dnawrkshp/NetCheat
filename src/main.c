#include "include/main.h"

/*
 *	Main function
 */
int main(int argc, char *argv[]) {
	int fdn, i = 0;
	
	run_path[0] = 0;
	strcpy(run_path, argv[0]);
	
	/* Transform the boot path to homebrew standards */	
	if (!strncmp(run_path, "mass0:", 6)) { /* SwapMagic boot path for usb_mass */
		run_path[4] = ':';
		strcpy(&run_path[5], &run_path[6]);

		for (i=0; run_path[i]!=0; i++) {
			if (run_path[i] == '\\')
				run_path[i] = '/';
		}
	}
	
	#ifndef DEBUG
	IOP_Reset();
	#endif
	
	SifInitRpc(0);
	
	/* Defaults to NTSC mode */
	TV_mode = GS_MODE_NTSC;
	
	/* Reading ROMVER */
	if((fdn = open("rom0:ROMVER", O_RDONLY)) > 0) {
		read(fdn, romver, sizeof romver);
		close(fdn);
	}

	/* Set Video PAL mode if needed */
	if (romver[4] == 'E') TV_mode = GS_MODE_PAL;
	//TV_mode = GS_MODE_NTSC; // to force NTSC on PAL console
	
	/* Set default screen values to use with GsKit depending on TV mode */
	if (TV_mode == GS_MODE_PAL) { /* PAL values fit perfectly on my TV */
		SCREEN_WIDTH  = 640;
		SCREEN_HEIGHT = 512;
		SCREEN_X	  = 692;
		SCREEN_Y	  = 72;
		Y_RATIO	  	  = 1.0f;
	} else { /* NTSC values can be adjusted, although it fit fine too on my TV */
		SCREEN_WIDTH  = 640;
		SCREEN_HEIGHT = 448;
		SCREEN_X 	  = 672;
		SCREEN_Y	  = 50;
		Y_RATIO	  	  = 0.875f;
	}

	/* needed before to init pads */
	TimerInit();

	/* setup GS */
	Setup_GS(TV_mode);
	
	/* Prior to call any of the gfx functions */
	gfx_set_defaults();
	
	/* Load Textures into VRAM */
	vram_free();
	load_background_Textures();	
	load_Font();
	
	Draw_WaitMenu(wLoad, wMax);
	
	LoadModules();
	
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
	
	i = LoadSettings();
	/*
	//Static IP setup
	i = 1;
	strcpy((char*)ip, "192.168.1.139");
	strcpy((char*)nm, "255.255.255.0");
	strcpy((char*)gw, "192.168.1.1");
	*/
	
	if (i >= 0) {
		
		LoadIRX();
		AnimateFade(0, 128, 1, 10000);
		
		sprintf((char*)ipString, "IP Address: %s", ip);
		UpdateMMenu("Not Connected!");
		
		/* Font spacing tests */
		/*
		UpdateMMenu(" !\"#$%&'()*+,-./0123456789:;<=>?"); //Rows 1 and 2
		UpdateMMenu("@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"); //Rows 3 and 4
		UpdateMMenu("`abcdefghijklmnopqrstuvwxyz{|}~?");  //Rows 5 and 6
		*/
	
		StartServer();
	} else {
		strcpy((char*)ipString, "Failed to load settings file from:");
		strcpy((char*)midS, "mc?:/SYS-CONF/IPCONFIG.DAT");
		UpdateMMenu((char*)midS);
		SleepThread();
	}
	
	return 0;
}

/*
 * Loads in all the modules
 */
int LoadModules(void) {
	int ret = 1;

	SifLoadFileInit();
	SifInitRpc(0);

	// Load modules off ROM
	ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
	wLoad++; Draw_WaitMenu(wLoad, wMax);
	ret = SifLoadModule("rom0:MCMAN", 0, NULL);
	wLoad++; Draw_WaitMenu(wLoad, wMax);
	ret = SifLoadModule("rom0:MCSERV", 0, NULL);
	wLoad++; Draw_WaitMenu(wLoad, wMax);
	ret = SifLoadModule("rom0:CDVDMAN", 0, NULL);
	///ret = SifLoadModule("rom0:LIBSD", 0, NULL);

	// Modules from PS2SDK
	SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret);
	wLoad++; Draw_WaitMenu(wLoad, wMax);
	SifExecModuleBuffer(&usb_mass_irx, size_usb_mass_irx, 0, NULL, &ret);
	wLoad++; Draw_WaitMenu(wLoad, wMax);
	
	// QWERTY Module from CogSwap
	loadmodulemem(iopmod, 1865, 0, 0);
	wLoad++; Draw_WaitMenu(wLoad, wMax);

	SifLoadFileExit();
	
	return ret;
}

#ifndef DEBUG //So there isn't a duplicate
/*
 * Loads the array at ptr into the memory at addr
 */
int InstallArray(int addr, void ** ptr, int size) {
	int i = 0;
	u32 Store = addr;
	u32 Read = (u32)ptr;
	
	if (addr >= 0x80000000) { /* Kernel */
		for (i = 0; i < size; i += 1)
		{
			DI();
			ee_kmode_enter();
			*(u8*)Store = *(u8*)Read;
			ee_kmode_exit();
			EI();
			Store += 1;
			Read += 1;
		}
	} else { /* EE */
		for (i = 0; i < size; i += 1)
		{
			*(u8*)Store = *(u8*)Read;
			Store += 1;
			Read += 1;
		}
	}
	
	#ifdef DEBUG
		printf("addr = %08x, ptr = %08x, size = %x (%d)\n", (int)addr, (int)ptr, (int)size, (int)size);
	#endif
	
	return 1;
}
#endif

/*
 * load an elf file using embedded elf loader
 * this allow to fix memory overlapping problem
 */
void load_elf(char *elf_path, int code_len) {
	
	u8 *boot_elf;
	elf_header_t *boot_header;
	elf_pheader_t *boot_pheader;
	int i = 0;
	char *args[1];
	
	if (elf_path == NULL) { return; }
	if (elf_path[strlen(elf_path) - 1] == '\n')
		elf_path[strlen(elf_path) - 1] = '\0'; //A simple fix to a bug with booting uLE
	
	/* The loader is embedded */
	boot_elf = (u8 *)&elf_loader;

	/* Get Elf header */
	boot_header = (elf_header_t *)boot_elf;

	/* Check elf magic */
	if ((*(u32*)boot_header->ident) != ELF_MAGIC)
		return;

	/* Get program headers */
	boot_pheader = (elf_pheader_t *)(boot_elf + boot_header->phoff);
	
	/* Scan through the ELF's program headers and copy them into appropriate RAM
	 * section, then pad with zeros if needed.
	 */
	for (i = 0; i < boot_header->phnum; i++) {
		if (boot_pheader[i].type != ELF_PT_LOAD)
			continue;

		memcpy(boot_pheader[i].vaddr, boot_elf + boot_pheader[i].offset, boot_pheader[i].filesz);

		if (boot_pheader[i].memsz > boot_pheader[i].filesz)
			memset(boot_pheader[i].vaddr + boot_pheader[i].filesz, 0, boot_pheader[i].memsz - boot_pheader[i].filesz);
	}

	#ifdef DEBUG
	Install_LD();																	/* Install live debug */
	
	int joker_addr = 0x001EE682;													/* preset joker address */
	DI();
	ee_kmode_enter();
		*(u32*)0x8007E7FC = (int)joker_addr;										/* 0 = use joker scanner */
		*(u32*)0x8007E810 = (int)joker_addr;										/* preset joker address */
	ee_kmode_exit();
	EI();
	#endif
	
	/* Setup engine */
	DI();
	ee_kmode_enter();
		memcpy((u32*)EngineAddr, NCEngine, sizeof(NCEngine));						/* Install the LiveDebug v3 Engine into the kernel */
		memset((u32*)CodesAddr, 0 , code_len + 8);									/* Clear code area */
		memcpy((u32*)(CodesAddr + 8), (u32*)0x000F0000, code_len);					/* Install the codes into the kernel */
		*(u32*)CodesAddr = (CodesAddr + 0x10); 										/* Pointer to current code */
		*(u32*)HookAddr = HookValue;												/* Install the kernel hook */
	ee_kmode_exit();
	EI();
	
	#ifdef DEBUG
	printf("Installed hook (0x%08x) at 0x%08x\n", HookValue, HookAddr);
	#endif
	
	/* Cleanup before launching elf loader */
	CleanUp();
	
	args[0] = elf_path;
	
	/* Execute Elf Loader */
	ExecPS2((void *)boot_header->entry, 0, 1, args);
}

int LoadIRX()
{

	int a;
	
	#ifndef DEBUG
	a = SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &a);
	if (a < 0 )
	{
	return -1;
	}

	wLoad++;
	Draw_WaitMenu(wLoad, wMax);
	
	a = SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, &a);
   	if (a < 0 )
	{
	return -1;
	}

	wLoad++;
	Draw_WaitMenu(wLoad, wMax);
	
	a = SifExecModuleBuffer(&ps2ip_irx, size_ps2ip_irx, 0, NULL, &a);
	if (a < 0 )
	{
	return -1;
	}
	
	wLoad++;
	Draw_WaitMenu(wLoad, wMax);
	
	#endif
	
	a = SifExecModuleBuffer(&dns_irx, size_dns_irx, 0, NULL, &a);
	if (a < 0)
	{
	return -1;
	}

	wLoad++;
	Draw_WaitMenu(wLoad, wMax);
	
	a = SifExecModuleBuffer(&ps2ips_irx, size_ps2ips_irx, 0, NULL, &a);
	if (a < 0)
	{
	return -1;
	}

	wLoad++;
	Draw_WaitMenu(wLoad, wMax);
	
	#ifndef DEBUG /* The IP of ps2link is used */
	//Taken from http://forums.ps2dev.org/viewtopic.php?t=11553&highlight=ps2ipinit
    //Credit to methos3
    char *args = malloc(strlen((char*)ip) + strlen((char*)nm) + strlen((char*)gw) + 3); 
    int b = 0; 
	memset(&b, 0, sizeof(b + 1));
	strcpy(&args[b], (char*)ip); b+=strlen((char*)ip)+1; 
	strcpy(&args[b], (char*)nm); b+=strlen((char*)nm)+1; 
	strcpy(&args[b], (char*)gw); b+=strlen((char*)gw)+1; 
	a = SifExecModuleBuffer(&smap_irx, size_smap_irx, b, args, &a);
	if (a < 0)
	{
	return -1;
	}
	
	wLoad++;
	Draw_WaitMenu(wLoad, wMax);
	
	#endif
	
	a = ps2ip_init();
	if (a < 0)
		return -1;
	
	wLoad++;
	Draw_WaitMenu(wLoad, wMax);
	
	return 0;
}

void spinCD(void) {
	/* From Cora Dumper */
	//cdTrayReq(0,0x00490000);
	
	//cdDiskReady(0);
	// Spin up the cd/dvd
	//cdStandby();
	
	//int fd = fioOpen(sys_cnf, O_RDONLY);
	//fioClose(fd);
}

//----------------------------------------------------------------
int     get_CNF_string(u8 **CNF_p_p, u8 **name_p_p, u8 **value_p_p) //Taken from uLaunchElf
{
        
        u8 *np, *vp, *tp = *CNF_p_p;

start_line:
        while((*tp<=' ') && (*tp>'\0')) tp+=1;  //Skip leading whitespace, if any
        if(*tp=='\0') return 0;                         //but exit at EOF
        np = tp;                                //Current pos is potential name
        if(*tp<'A')                             //but may be a comment line
        {                                       //We must skip a comment line
                while((*tp!='\r')&&(*tp!='\n')&&(*tp>'\0')) tp+=1;  //Seek line end
                goto start_line;                    //Go back to try next line
        }

        while((*tp>='A')||((*tp>='0')&&(*tp<='9'))) tp+=1;  //Seek name end
        if(*tp=='\0') return -1;                        //but exit at EOF

        while((*tp<=' ') && (*tp>'\0'))
                *tp++ = '\0';                       //zero&skip post-name whitespace
        if(*tp!='=') return -2;                 //exit (syntax error) if '=' missing
        *tp++ = '\0';                           //zero '=' (possibly terminating name)

        while((*tp<=' ') && (*tp>'\0')          //Skip pre-value whitespace, if any
                && (*tp!='\r') && (*tp!='\n')           //but do not pass the end of the line
                )tp+=1;                                                         
        if(*tp=='\0') return -3;                        //but exit at EOF
        vp = tp;                                //Current pos is potential value

        while((*tp!='\r')&&(*tp!='\n')&&(*tp!='\0')) tp+=1;  //Seek line end
        if(*tp!='\0') *tp++ = '\0';             //terminate value (passing if not EOF)
        while((*tp<=' ') && (*tp>'\0')) tp+=1;  //Skip following whitespace, if any

        *CNF_p_p = tp;                          //return new CNF file position
        *name_p_p = np;                         //return found variable name
        *value_p_p = vp;                        //return found variable value
        return 1;                               //return control to caller
}       //Ends get_CNF_string

//----------------------------------------------------------------
int ParseSYSTEMCNF(char *system_cnf, char *boot_path) //Taken from IGmassdumper
{
        int r, entrycnt, cnfsize;
        u8 *pcnf, *pcnf_start, *name, *value;
        int fd = -1;
        
        fd = open(system_cnf, O_RDONLY);
        if (fd < 0)
                return -1;              

        cnfsize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        
        pcnf = (char *)malloc(cnfsize);
        pcnf_start = pcnf;
        if (!pcnf) {
                close(fd);
                return -2;
        }
        
        r = read(fd, pcnf, cnfsize);    // Read CNF in one pass
        if (r != cnfsize)
                return -3;
                
        close(fd);
        pcnf[cnfsize] = 0;                              // Terminate the CNF string
        
        for (entrycnt = 0; get_CNF_string(&pcnf, &name, &value); entrycnt++) {
                if (!strcmp(name,"BOOT2"))  // check for BOOT2 entry
                        strcpy(boot_path, value);                       
        }
                        
        pcnf = pcnf_start;
        free(pcnf);
        
        return 1;
}

char *ReadIP()
{

	int fp, fpSize;
	char *result = NULL;
	fp = fioOpen("mc0:/IP.txt", O_RDONLY);

	if (fp < 0)
	{
		#ifdef DEBUG
			printf("	mc0:/IP.txt does not exist!\n");
		#endif
	}
	else
	{

	fpSize = fioLseek(fp, 0, SEEK_END);
	fioLseek(fp, 0, SEEK_SET);
	
	result = malloc(fpSize + 1);
	fioRead(fp, result, fpSize);
	fioClose(fp);
	
}

	return result;

}

char *ReadNM()
{

	int fp, fpSize;
	char *result = NULL;
	fp = fioOpen("mc0:/NM.txt", O_RDONLY);

	if (fp < 0)
	{
		
	}
	else
	{

	fpSize = fioLseek(fp, 0, SEEK_END);
	fioLseek(fp, 0, SEEK_SET);
	
	result = malloc(fpSize + 1);
	fioRead(fp, result, fpSize);
	fioClose(fp);
	
}
	return result;

}

char *ReadGW()
{

	int fp, fpSize;
	char *result = NULL;
	fp = fioOpen("mc0:/GW.txt", O_RDONLY);

	if (fp < 0)
	{
	}
	else
	{

	fpSize = fioLseek(fp, 0, SEEK_END);
	fioLseek(fp, 0, SEEK_SET);
	
	result = malloc(fpSize + 1);
	fioRead(fp, result, fpSize);
	fioClose(fp);
	
}

	return result;

}

void UpdateMMenu(char *text) {
		strcpy((char*)midString, (char*)text);
		while (!Draw_MainMenu())
			Render_GUI();
		Render_GUI();
}

int StartServer() {
	
	int n = 0;
	
	//ip_addr ip;
	//cdTrayReq(0, (u32*)0x00490000);
	
	int sockfd, newsockfd, portno = 2345;
    socklen_t clilen;
	
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (sockfd < 0)
	{
		SleepThread();
	}

	//int ps2ip_setconfig(ipinfo);

	memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portno);
	
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
	{
		return -1;
	}
	
	//setsockopt(sockfd, IPPROTO_TCP, O_NONBLOCK, 0, int optlen);
	
	/*
	scr_printf("	Port: %d\n", portno);\
	
	scr_printf("	IP: %s\n", "10.0.1.100");//ReadIP());

	scr_printf("	Listening for any incoming connection...\n");
	*/
	
	while ( 1 ) //If sudden disconnect, the loop will reset and wait for the PC to reconnect
	{
		
		char *buffer = (char*)malloc(1024); //1 kilobyte
		memset((char*)buffer, 0, sizeof(buffer));
		char *buf_codes = NULL;
		
		listen(sockfd, 5);
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	
		char belf[40];
		//ParseSYSTEMCNF(sys_cnf, (char*)belf);
		
		UpdateMMenu("Connected");
		
		//char codewait[] = "(PS2) Waiting for codes!\n";
		//char recvcode[] = "(PS2) Codes recieved!\n";
		char startgm[1024];
		//char altboot[1024];
		//sprintf(altboot, "(PS2) Booting %s with method 2\n", (char*)belf);
		int code_len = 0;
		
		if (newsockfd < 0) 
		{
			
		}
		
		z = 0;
		//send(newsockfd, codewait, strlen(codewait), 0);	
		
		while ( z >= 0 ) /* The PC will send a packet containing "Dis" when it closes or disconnects. It will then set z < 0 on disconnect. */
		{
			
			n = _recv(newsockfd, buffer, 2, 0, 0);
			if (n < 0) { n = -1; } else { buffer[1] = '\0'; }
			n = 0;

			//n = send(newsockfd,"Recieved!\n", 18, 0);	
		
			//scr_printf("	Recieved: %s\n", buffer);
		
			if ( strcmp(buffer, "0") == 0) /* Start game */
			{	
				ParseSYSTEMCNF(sys_cnf, belf);
				if( (char*)belf == NULL)
				{
					#ifdef DEBUG
						printf("	Could not read system.cnf\n");
					#endif
					SleepThread();
				}
				
				sprintf(startgm, "Booting %s\n", (char*)belf);
				z = _send(newsockfd, startgm, strlen(startgm), 0, 0);
				
				close(newsockfd);
				close(sockfd);
				
				AnimateFade(128, 0, -1, 10000);
			
				load_elf((char*)belf, code_len);
				//load_elf("mc0:/ps2link/ps2link.elf");
				//ExecGame(0, elf, 0x80030100);
			} else if (strcmp(buffer, "1") == 0) { /* Disconnect */
				z = -1;
			} else if (strcmp(buffer, "2") == 0) { /* Receive codes */
				char *temp = "Send em!";
				z = _send(newsockfd, temp, strlen(temp), 0, 0);
				
				_recv(newsockfd, buffer, 10, 0, 0);
				code_len = atoi(buffer);
				
				ReplyWithOkay(newsockfd);
				
				buf_codes = (char*)malloc(code_len + 8);
				memset((char*)buf_codes, 0, code_len + 8);
				
				_recv(newsockfd, buf_codes, code_len, 0, 2);
				temp = "Recieved codes!";
				//printf("buffer:\n%s\n", buffer);
				memcpy((u32*)0x000F0000, buf_codes, code_len);
				free(buf_codes);
				//printf("\na = %i\nbuffer: %s\n", a, buffer);
				
				z = _send(newsockfd, temp, strlen(temp), 0, 0);
				
				wait_for_okay(newsockfd);
				#ifdef DEBUG
					printf("Recieved %i lines of code (%.02f kb)\n", code_len / 8, (float)code_len / 1024);
				#endif
				
				sprintf((char*)midS, "Recieved %i lines of code (%.02f kb)", code_len / 8, (float)code_len / 1024);
				UpdateMMenu((char*)midS);
				
				//scr_printf("\tRecieved %i lines of code (%.02f kb)\n", code_len / 8, (float)code_len / 1024);
				
			} else if ( strcmp(buffer, "3") == 0 ) { /* Boot custom boot path */
				
				
				memset((char*)buffer, 0, 100);
				_recv(newsockfd, buffer, 100, 0, 0);
				
				close(newsockfd);
				close(sockfd);
				
				AnimateFade(128, 0, -1, 10000);
			
				load_elf((char*)buffer, code_len);
			} else if ( strcmp(buffer, "4") == 0 ) { /* Stop Disc */
				
				StopDisc();
				
				ReplyWithOkay(newsockfd);
				
			}
		
		}
		
		free(buffer);
		close(newsockfd);
		
		UpdateMMenu("Not Connected!");
		
		//scr_printf("	IP: %s\n", ReadIP()); /* Print out IP in case of sudden disconnect */
		//scr_printf("	Waiting for connection...\n"); /* Prints that the ps2 is listening for a connection, loops back to the main loop */
		
		}
		
    close(newsockfd);
    close(sockfd);
	
	return 0;
	
}

void StopDisc(void)
{
	/*
	loadmodulemem(iopmod, 1865, 0, 0);
	wLoad++; Draw_WaitMenu(wLoad, wMax);
	*/
	fioOpen("qwerty:r",1); // Wait for the disc to be ready
	
	/* It seems I don't need this... hehe */
	/*
	ResetCNF();
	IOP_Reset();

	SifInitRpc(0);
	LoadModules();
	//LoadIRX();
	*/

	fioOpen("qwerty:c",1); // Set LBA to 0
	fioOpen("qwerty:s",1); // Stop the disc
}

void IOP_Reset(void)
{
  	while(!SifIopReset("rom0:UDNL rom0:EELOADCNF",0));
  	while(!SifIopSync());
  	fioExit();
  	SifExitIopHeap();
  	SifLoadFileExit();
  	SifExitRpc();

  	SifInitRpc(0);
  	FlushCache(0);
  	FlushCache(2);
}

void ResetCNF(void)
{
	fioOpen("cdrom0:\\SYSTEM.CNF;1",1);
	SifIopReset("rom0:UDNL cdrom0:\\SYSTEM.CNF;1", 0);

	while ( SifIopSync()) ;

	fioExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();
	EI();

	SifInitRpc(0);

	FlushCache(0);
	FlushCache(2);
}

void CleanUp(void)
{
	TimerEnd();
	
  	fioExit();
  	SifExitIopHeap();
  	SifLoadFileExit();
  	SifExitRpc();

  	FlushCache(0);
  	FlushCache(2);
	
	Clear_Screen();
}

int _recv(int s, char *buf, int len, int flags, int mode) {
	int x = 0, y = 1;
	
	switch (mode) {
		case 0: // recv in one go
			return recv(s, buf, len, flags);
		case 1: // recv byte by byte
			while (y != 0) {
				y = recv(s, buf + x, 1, flags);
				x += y;
			}
			return x;
		case 2: // recv byte by byte until len is reached
			while (x < len) {
				x += recv(s, buf + x, 1, flags);
			}
			return x;
	}
	return 0;
}

//send (int s, const void *msg, size_t len, int flags);
int _send(int s, char *buf, int len, int flags, int mode) {
	int x = 0, off = 0;
	
	switch (mode) {
		case 0: // send in one go
			return send(s, buf, len, flags);
			break;
		case 1: // send byte by byte
			while (x <= len) {
				x += send(s, buf + x, 1, flags);
			}
			break;
		case 2: // send in chunks specified by len
			while ((x = send(s, buf + off, len, flags)) > 0) {
				off += x;
				len -= x;
			}
	}
	return 0;
}

void wait_for_okay(int s) {
	char temp[6];
	do {
		recv(s, temp, 5, 0);
	} while ( strcmp(temp, "Okay") != 0 ); //Loops until "Okay" is recieved
}

int ReplyWithOkay(int s) {
	return _send(s, (char*)rep, strlen((char*)rep), 0, 1);
}

int LoadSettings(void) {
	
	/* From OPL's config.c */
	int ps2_ip[4], ps2_nm[4], ps2_gw[4];
	char ipconfig[255];
	
	int fd = fioOpen("mc0:SYS-CONF/IPCONFIG.DAT", O_RDONLY);
	if (fd >= 0) {
		int size = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);
		
		fioRead(fd, &ipconfig, size);
		fioClose(fd);
	} else if ((fd = fioOpen("mc1:SYS-CONF/IPCONFIG.DAT", O_RDONLY)) >= 0) {
		int size = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);
		
		fioRead(fd, &ipconfig, size);
		fioClose(fd);		
	} else { return -1; }
	
	sscanf(ipconfig, "%d.%d.%d.%d %d.%d.%d.%d %d.%d.%d.%d", &ps2_ip[0], &ps2_ip[1], &ps2_ip[2], &ps2_ip[3],
			&ps2_nm[0], &ps2_nm[1], &ps2_nm[2], &ps2_nm[3],
			&ps2_gw[0], &ps2_gw[1], &ps2_gw[2], &ps2_gw[3]);
	
	/* Move into global network variables */
	sprintf((char*)ip, "%d.%d.%d.%d", ps2_ip[0], ps2_ip[1], ps2_ip[2], ps2_ip[3]);
	sprintf((char*)nm, "%d.%d.%d.%d", ps2_nm[0], ps2_nm[1], ps2_nm[2], ps2_nm[3]);
	sprintf((char*)gw, "%d.%d.%d.%d", ps2_gw[0], ps2_gw[1], ps2_gw[2], ps2_gw[3]);
	#ifdef DEBUG
	printf("IP: %s\nNM: %s\nGW: %s\n", (char*)ip, (char*)nm, (char*)gw);
	#endif
	
	wLoad++;
	Draw_WaitMenu(wLoad, wMax);
	
	return 0;
}

/*
 * asm delay function
 */
void delay(int count) {
	int i;
	int ret;
	
	for (i  = 0; i < count; i++) {
		ret = 0x01000000;
		while(ret--)
			asm("nop\nnop\nnop\nnop");
	}
}

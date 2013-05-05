EE_BIN = NetCheat.elf
EE_BIN_PACKED = NC.elf
EE_BIN_DIR = NetCheat
IRX = $(PS2SDK)/iop/irx
IP_ADDR = 10.0.1.100
GUI = gui

EE_OBJS = src/main.o src/gui.o src/timer.o
EE_OBJS += usbd.o usbhdfsd.o
EE_OBJS += ps2smap.o ps2ip.o ps2ips.o ps2dev9.o dns.o poweroff.o
EE_OBJS += elf_loader.o
EE_OBJS += background.o logo.o font.o

EE_SRC = usbd.s usbhdfsd.s
EE_SRC += ps2smap.s ps2ip.s ps2ips.s ps2dev9.s dns.s poweroff.s
EE_SRC += elf_loader.s
EE_SRC += background.s logo.s font.s

EE_INCS = -I$(PS2DEV)/gsKit/include -I$(PS2SDK)/ports/include -I$(PS2SDK)/sbv/include
EE_LDFLAGS = -nostartfiles -L$(PS2DEV)/gsKit/lib -L$(PS2SDK)/ports/lib \
	-L$(PS2SDK)/sbv/lib -L. -s
EE_LIBS = -Xlinker --start-group -lps2ip -lpatches -lgskit -ldmakit -lm -lz -lpng -ldebug -lkernel -Xlinker --end-group


all: $(EE_BIN)
	ps2_packer/ps2_packer -p zlib $(EE_BIN) $(EE_BIN_PACKED)
	cp -f --remove-destination $(EE_BIN_PACKED) $(EE_BIN_DIR)/$(EE_BIN)
	make cleanup
 
exec:
	ps2client -h $(IP_ADDR) execee "host:$(EE_BIN_DIR)/$(EE_BIN)"
 
run:	
	make clean
	make
	ps2client -h $(IP_ADDR) execee "host:$(EE_BIN_DIR)/$(EE_BIN)"

reset:
	ps2client -h $(IP_ADDR) reset
 
clean:
	$(MAKE) -C elf_loader clean
	$(MAKE) -C smap clean
	rm -f $(EE_OBJS) $(EE_SRC) *.elf *.ELF

cleanup:
	$(MAKE) -C elf_loader clean
	$(MAKE) -C smap clean
	rm -f $(EE_OBJS) $(EE_SRC) $(EE_BIN) $(EE_BIN_PACKED)

#IRX Modules
usbd.s:
	bin2s $(PS2SDK)/iop/irx/usbd.irx usbd.s usbd_irx
usbhdfsd.s:
	bin2s $(PS2DEV)/usbhdfsd/bin/usbhdfsd.irx usbhdfsd.s usb_mass_irx
dns.s:
	bin2s $(IRX)/dns.irx dns.s dns_irx
poweroff.s:
	bin2s $(IRX)/poweroff.irx poweroff.s poweroff_irx
ps2dev9.s:
	bin2s $(IRX)/ps2dev9.irx ps2dev9.s ps2dev9_irx
ps2ip.s:
	bin2s $(IRX)/ps2ip.irx ps2ip.s ps2ip_irx
ps2ips.s:
	bin2s $(IRX)/ps2ips.irx ps2ips.s ps2ips_irx
ps2smap.s:
	$(MAKE) -C smap
	bin2s smap/ps2smap.irx ps2smap.s smap_irx

#GUI
background.s:
	bin2s $(GUI)/background.png background.s background
logo.s:
	bin2s $(GUI)/logo.png logo.s logo
font.s:
	bin2s $(GUI)/font.png font.s font

#ELF Loader
elf_loader.s:
	$(MAKE) -C elf_loader
	bin2s elf_loader/elf_loader.elf elf_loader.s elf_loader

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

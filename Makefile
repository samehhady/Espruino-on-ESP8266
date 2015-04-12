# tnx to mamalala
# Changelog
# Changed the variables to include the header file directory
# Added global var for the XTENSA tool root
#
# This make file still needs some work.
#
#
# Output directors to store intermediate compiled files
# relative to the project directory
include CustomVariables
BUILD_BASE	= build
FW_BASE		= firmware
JS_BASE		= scripts

#ESPDELAY indicates seconds to wait between flashing the two binary images
ESPDELAY	?= 1
#ESPBAUD		?= 460800
ESPBAUD		?= 115200

# name for the target project
TARGET		= espruino

# which modules (subdirectories) of the project to include in compiling
#MODULES		= driver user lwip/api lwip/app lwip/core lwip/core/ipv4 lwip/netif
MODULES		= user
EXTRA_INCDIR	= include $(SDK_BASE)/include

# libraries used in this project, mainly provided by the SDK
LIBS		= c gcc hal phy pp net80211 lwip wpa upgrade main ssl

# compiler flags using during compilation of source files
CFLAGS		= -Os -std=gnu99 -fno-builtin -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

#-Wl,--size-opt

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# we create two different files for uploading into the flash
# these are the names and options to generate them
FW_FILE_1	= 0x00000
FW_FILE_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec
FW_FILE_2	= 0x10000
FW_FILE_2_ARGS	= -es .irom0.text $@ -ec
JS_FILE	= main

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-gcc
OC      := $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-objcopy
OD      := $(XTENSA_TOOLS_ROOT)xtensa-lx106-elf-objdump

####
#### no user configurable options below here
####
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR))
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

#LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))
LD_SCRIPT	:= $(addprefix -T./ld/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2).bin)
JS_FILE   :=$(addprefix $(JS_BASE)/,$(JS_FILE).js)

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
# move all object files to irom
	$(Q) $(OC) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)

$(FW_FILE_1): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_1_ARGS)

$(FW_FILE_2): $(TARGET_OUT) firmware
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_2_ARGS)

$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
#	$(vecho) $(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
#	$(Q) $(OD) -h build/user/jsvar.o build/user/user_main.o $@
	$(Q) $(OD) -h $@

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

flash: $(FW_FILE_1) $(FW_FILE_2)
#	$(Q) $(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) -ca 0x00000 -cf firmware/0x00000.bin -v
	$(Q) [ $(ESPDELAY) -ne 0 ] && echo "Please put the ESP in bootloader mode..." || true
	$(Q) sleep $(ESPDELAY) || true
	$(Q) $(XTENSA_TOOLS_ROOT)esptool.py --port $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin

js: $(JS_FILE)
	$(Q) [ $(ESPDELAY) -ne 0 ] && echo "Please put the ESP in bootloader mode..." || true
	@echo $(filter-out $@,$(MAKECMDGOALS))
	$(Q) sleep $(ESPDELAY) || true
	$(Q) $(XTENSA_TOOLS_ROOT)esptool.py --port $(ESPPORT) write_flash 0x60000 $(JS_BASE)/$(filter-out $@,$(MAKECMDGOALS)).js

#webpages.espfs: html/ html/wifi/ mkespfsimage/mkespfsimage
webpages: html/ html/wifi/ mkespfsimage/mkespfsimage
#	cd html; find * | ../mkespfsimage/mkespfsimage > ../webpages.espfs; cd ..
	cd html; find * | ../mkespfsimage/mkespfsimage-mac > ../webpages.espfs; cd ..

mkespfsimage/mkespfsimage: mkespfsimage/
	make -C mkespfsimage

htmlflash: webpages.espfs
#	if [ $$(stat -c '%s' webpages.espfs) -gt $$(( 0x2E000 )) ]; then echo "webpages.espfs too big!"; false; fi
#	$(ESPTOOL) -cp $(ESPPORT) -cb $(ESPBAUD) -ca 0x12000 -cf webpages.espfs -v
	$(Q) [ $(ESPDELAY) -ne 0 ] && echo "Please put the ESP in bootloader mode..." || true
	$(Q) sleep $(ESPDELAY) || true
	$(Q) $(XTENSA_TOOLS_ROOT)esptool.py --port $(ESPPORT) write_flash 0x12000 webpages.espfs

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) find $(BUILD_BASE) -type f | xargs rm -f

	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
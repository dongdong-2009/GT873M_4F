
include ../makefile.rc
MOD_NAME=ptp
MOD_TARGET=library
MOD_SRC_HOME=$(SWHOME)/src

ECOS_INSTALL_HOME=../iros/install
IROS_CONFIG_H=$(shell pwd)/../iros_config.h

MOD_CFLAGS= -Werror -Wno-uninitialized -nostdlib -fno-inline \
-mthumb-interwork -mthumb -mcpu=arm9 \
-include $(IROS_CONFIG_H)

CFILES= \
#$(SWHOME)/src/onu/apps/lynx/ptp/ptp_halftc.c    \
#$(SWHOME)/src/onu/apps/lynx/ptp/ptp_parse.c     \
#$(SWHOME)/src/onu/apps/lynx/ptp/ptp_cmd.c     \
#$(SWHOME)/src/onu/apps/lynx/ptp/ptp_mgmt.c      \
#$(SWHOME)/src/onu/apps/lynx/ptp/ptp_lynx.c      \
#$(SWHOME)/src/onu/apps/lynx/ptp/802_1as.c



MOD_INCLUDES=-I$(ECOS_INSTALL_HOME)/include \
-I$(SWHOME)/src/cmn/plat/include \
-I$(SWHOME)/src/cmn/plat/osal/ecos \
-I$(SWHOME)/src/cmn/plat/sdl \
-I$(SWHOME)/src/cmn/plat/peripheral \
-I$(SWHOME)/src/cmn/apps/oam/include \
-I$(SWHOME)/src/cmn/apps/core \
-I$(SWHOME)/src/cmn/apps/event \
-I$(SWHOME)/src/include \
-I$(SWHOME)/src/onu/plat/lynxd \
-I$(SWHOME)/src/onu/sdl/include \
-I$(SWHOME)/src/onu/sdl/include/cmn \
-I$(SWHOME)/src/onu/sdl/include/ctc \
-I$(SWHOME)/src/onu/aal/lynxd/include \
-I$(SWHOME)/src/onu/aal/lynxd/ \
-I$(SWHOME)/src/onu/apps/lynx/cortina \
-I$(SWHOME)/src/onu/apps/lynx/oam/ctc \
-I$(SWHOME)/src/onu/apps/lynx/oam \
-I$(SWHOME)/src/onu/cmd \
-I$(SWHOME)/src/onu/apps/lynx/gpio/cmn \
-I$(SWHOME)/src/onu/apps/lynx/ptp

include $(SWHOME)/tools/support/build/makefile.mod_start


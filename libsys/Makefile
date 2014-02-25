
include ../makefile.rc
MOD_NAME=sys
MOD_TARGET=library
MOD_SRC_HOME=$(SWHOME)/src

ECOS_INSTALL_HOME=../iros/install
IROS_CONFIG_H=$(shell pwd)/../iros_config.h

MOD_CFLAGS= -Werror -Wno-uninitialized -nostdlib -fno-inline \
-mthumb-interwork -mthumb -mcpu=arm9 -DUSE_SEMAPHORE -D_RELEASE_VERSION_\
-include $(IROS_CONFIG_H)

CFILES= \
$(SWHOME)/src/cmn/apps/event/event.c \
$(SWHOME)/src/onu/apps/lynx/cortina/frame.c \
$(SWHOME)/src/onu/apps/lynx/cortina/hello.c \
$(SWHOME)/src/onu/apps/lynx/cortina/l2ftp.c \
$(SWHOME)/src/onu/apps/lynx/cortina/config.c \
$(SWHOME)/src/onu/apps/lynx/cortina/app_init.c \
$(SWHOME)/src/onu/apps/lynx/cortina/app_evt.c \
$(SWHOME)/src/onu/apps/lynx/cortina/app_timer.c \
$(SWHOME)/src/onu/apps/lynx/cortina/app_vlan.c \
$(SWHOME)/src/onu/apps/lynx/vlan/app_vlan_ctc.c \
$(SWHOME)/src/onu/apps/lynx/cortina/app_cmd.c \
$(SWHOME)/src/onu/apps/lynx/cortina/img_upgrade.c \
$(SWHOME)/src/onu/apps/lynx/cortina/app_daemon.c \
$(SWHOME)/src/onu/apps/lynx/cortina/packet.c \
$(SWHOME)/src/onu/apps/lynx/cortina/port_stats.c \
$(SWHOME)/src/onu/apps/gwd/rcp_gwd.c \
$(SWHOME)/src/onu/apps/gwd/rcp_switch_cli.c \
$(SWHOME)/src/onu/apps/gwd/onu_printf.c \
$(SWHOME)/src/onu/apps/gwd/onu_uax.c \
$(SWHOME)/src/onu/apps/gwd/oam.c \
$(SWHOME)/src/onu/apps/gwd/gwd_eth_loop_detect.c \
$(SWHOME)/src/onu/apps/gwd/timer.c \
$(SWHOME)/src/onu/apps/gwd/gwd_broadstorm.c \
$(SWHOME)/src/onu/3rd/driver/marvell/msApiInit.c \
$(SWHOME)/src/onu/3rd/driver/marvell/iros_mii.c \
$(SWHOME)/src/onu/3rd/driver/marvell/osSem.c \
$(SWHOME)/src/onu/3rd/driver/marvel/switch_drv.c \
$(SWHOME)/src/onu/3rd/driver/marvel/switch.c \
#$(SWHOME)/src/onu/apps/gwd/iros_flash_oper.c


MOD_INCLUDES=-I$(ECOS_INSTALL_HOME)/include \
-I$(SWHOME)/src/cmn/plat/include \
-I$(SWHOME)/src/cmn/plat/osal/ecos \
-I$(SWHOME)/src/cmn/plat/sdl \
-I$(SWHOME)/src/cmn/apps/oam/include \
-I$(SWHOME)/src/cmn/apps/core \
-I$(SWHOME)/src/cmn/apps/event \
-I$(SWHOME)/src/cmn/apps/mcast/inc \
-I$(SWHOME)/src/onu/apps/lynx/mcast \
-I$(SWHOME)/src/onu/apps/lynx/pppoe \
-I$(SWHOME)/src/include \
-I$(SWHOME)/src/onu/plat/lynxd \
-I$(SWHOME)/iros/install/include/lwip \
-I$(SWHOME)/src/onu/plat/lynxd/mports/ctc \
-I$(SWHOME)/src/onu/sdl/include \
-I$(SWHOME)/src/onu/sdl/include/cmn \
-I$(SWHOME)/src/onu/sdl/include/ctc \
-I$(SWHOME)/src/onu/aal/lynxd/include \
-I$(SWHOME)/src/onu/aal/lynxd/ \
-I$(SWHOME)/src/onu/apps/lynx/cortina \
-I$(SWHOME)/src/onu/apps/lynx/oam/ctc \
-I$(SWHOME)/src/onu/apps/lynx/oam/cs \
-I$(SWHOME)/src/onu/apps/lynx/dbmngt/ctc \
-I$(SWHOME)/src/onu/apps/lynx/oam \
-I$(SWHOME)/src/onu/apps/lynx/dbmngt/cmn \
-I$(SWHOME)/src/onu/apps/lynx/loopdetect \
-I$(SWHOME)/src/onu/apps/lynx/trmsvr \
-I$(SWHOME)/src/onu/apps/lynx/ptp \
-I$(SWHOME)/src/onu/apps/lynx/web/cmn/cgi/include/ \
-I$(SWHOME)/src/onu/cmd \
-I$(SWHOME)/src/onu/apps/gwd \
-I$(SWHOME)/src/onu/apps/lynx/gpio/cmn \
-I$(SWHOME)/src/onu/3rd/driver/realtek \
-I$(SWHOME)/src/cmn/plat/include \
-I$(SWHOME)/src/cmn/plat/peripheral \
-I$(SWHOME)/src/cmn/apps/mcast/inc \
-I$(SWHOME)/src/onu/sdl/cmn/cmn \
-I$(SWHOME)/src/onu/sdl/mports/ctc \
-I$(SWHOME)/gtmrv/include \
-I$(SWHOME)/gtmrv/include/h/driver \
-I$(SWHOME)/gtmrv/include/h/msApi \
-I$(SWHOME)/gtmrv/include/h/platform



include $(SWHOME)/tools/support/build/makefile.mod_start


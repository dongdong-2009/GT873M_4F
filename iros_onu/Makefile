
include ../makefile.rc

MOD_NAME=iros_onu_asic_ram_lynxd
MOD_SRC_HOME=$(SWHOME)/src/cmn/plat
MOD_TARGET=application

INSTALL_HOME=$(HOME)/../projects/iros/rels/onu
INSTALL_NAME_REFIX=onu-arm9-
INSTALL_LIST=iros_onu_asic_ram_lynxd.blob iros_onu_asic_ram_lynxd.img

ECOS_INSTALL_HOME=../iros/install

MOD_CFILES= \
$(SWHOME)/src/cmn/plat/main.c \
$(SWHOME)/src/onu/apps/lynx/web/cmn/cgi/cgi_handler.c \
$(SWHOME)/src/onu/apps/lynx/web/cs/cgi/cgi_cs_handler.c \


CFILES=$(MOD_CFILES)

MOD_INCLUDES= \
-I$(ECOS_INSTALL_HOME)/include \
-I$(SWHOME)/src/cmn/plat/include \
-I$(SWHOME)/src/cmn/plat/osal/ecos \
-I$(SWHOME)/src/cmn/plat/sdl \
-I$(SWHOME)/src/include \
-I$(SWHOME)/src/onu/plat/lynxd \
-I$(SWHOME)/src/onu/apps/lynx/trmsvr \
-I$(SWHOME)/src/onu/aal/lynxd/include \
-I$(SWHOME)/src/onu/sdl/include \
-I$(SWHOME)/src/onu/sdl/include/cmn \
-I$(SWHOME)//src/onu/apps/lynx/dbmngt/ctc/ \
-I$(SWHOME)//src/onu/apps/lynx/dbmngt/cmn/ \
-I$(SWHOME)//src/onu/apps/lynx/web/cmn/cgi/include \
-I$(SWHOME)/src/onu/apps/lynx/web/cs/cgi/include \
-I$(SWHOME)/src/onu/apps/lynx/gpio/cmn


IROS_CONFIG_H=$(shell pwd)/../iros_config.h
MOD_CFLAGS= -Werror -nostdlib -Wno-uninitialized \
-mthumb -mthumb-interwork -mcpu=arm9 \
-include $(IROS_CONFIG_H)


MOD_LDFLAGS= -u imst_cli_start -u netdev_eth0 -u flash_hwr_init \
-L$(ECOS_INSTALL_HOME)/lib \
-L../libosal \
-L../libplat \
-L../libcache \
-L../libflash \
-L../libipintf \
-L../libweb \
-L../libsys \
-L../libtrmsvr \
-L../liboam \
-L../libdb \
-L../libloopdetect \
-L../libaal \
-L../libswitch \
-L../libmc	\
-L../libptp	\
-L../libpppoe  \
-L../libsdl	\
-lplat \
-lipintf \
-lsys \
-loam \
-lloopdetect \
-ldb \
-ltrmsvr \
-lweb \
-lmc    \
-lptp	\
-lpppoe \
-lsdl	\
-laal \
-lswitch \
-lflash \
-lcache \
-lplat \
-lsys \
-losal \
-Wl,-gc-sections \
-Ttarget.ld -nostdlib -mcpu=arm9 -mthumb-interwork \
-Wl,--Map -Wl,iros_onu_asic_ram_lynxd.map


TARGET=$(MOD_TMPHOME)/$(MOD_NAME).zblob $(MOD_TMPHOME)/$(MOD_NAME).wfs $(MOD_TMPHOME)/$(MOD_NAME).wlb
CLEAN_FILES=$(MOD_TMPHOME)/$(MOD_NAME).zimg $(TARGET) $(MOD_TMPHOME)/$(MOD_NAME)*.bin $(MOD_TMPHOME)/$(MOD_NAME).jffs2 $(TEMP_WEB_ROOT_DIR)

IMAGES=$(MOD_TMPHOME)/$(MOD_NAME).zblob $(MOD_TMPHOME)/$(MOD_NAME).wlb  $(MOD_TMPHOME)/$(MOD_NAME).wfs
IMAGES_DIR=$(MOD_TMPHOME)/../images/



include $(SWHOME)/tools/support/build/makefile.mod_start
$(MOD_TMPHOME)/$(MOD_NAME): ../libosal/libosal.a ../libplat/libplat.a ../libcache/libcache.a ../libmc/libmc.a ../libflash/libflash.a ../libipintf/libipintf.a ../libweb/libweb.a  ../libsys/libsys.a ../liboam/liboam.a  ../libdb/libdb.a  ../libtrmsvr/libtrmsvr.a ../libloopdetect/libloopdetect.a ../libaal/libaal.a ../libswitch/libswitch.a ../libsdl/libsdl.a ../libpppoe/libpppoe.a

WEB_CMN_ROOT=$(SWHOME)/src/onu/apps/lynx/web/cmn/res
WEB_CTC_ROOT=$(SWHOME)/src/onu/apps/lynx/web/cs/res/
WEB_RES_FILES=$(shell find  ${WEB_CMN_ROOT} ${WEB_CTC_ROOT}   -name "*")
WEB_ROOT_DIR=web_res

$(MOD_TMPHOME)/$(MOD_NAME).wfs: $(WEB_RES_FILES)  $(MOD_TMPHOME)/$(MOD_NAME).zblob
	mkdir ${WEB_ROOT_DIR} -p
	cp $(WEB_CMN_ROOT)/* $(WEB_ROOT_DIR)/ -a
	cp $(WEB_CTC_ROOT)/* $(WEB_ROOT_DIR)/ -a
	find $(WEB_ROOT_DIR)/ -name "CVS" |xargs rm -rf 
	mkfs.jffs2 -l -q -d $(WEB_ROOT_DIR)
	mv web_res.jffs2.wfs $(MOD_NAME).wfs
	mv web_res.jffs2 $(MOD_NAME).jffs2

$(MOD_TMPHOME)/$(MOD_NAME).wlb: $(MOD_TMPHOME)/$(MOD_NAME).zblob
	httpimg $(MOD_TMPHOME)/$(MOD_NAME).zblob
	mv $(MOD_TMPHOME)/$(MOD_NAME).zblob.wlb  $(MOD_TMPHOME)/$(MOD_NAME).wlb


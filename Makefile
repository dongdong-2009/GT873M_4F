
include makefile.rc

MAKEOPTS := SWHOME=$(SWHOME) \
            TOOLSHOME=$(TOOLSHOME) \
            BUILDID=$(BUILDID)
            
#USING_SW_TYPE:=MRV
#export USING_SW_TYPE           

TARGET_LIST= iros gtmrv libosal libplat libcache libmc libptp libflash libipintf libsys libtrmsvr liboam libweb libdb  libaal libswitch libsdl  libloopdetect libpppoe iros_onu
include $(SWHOME)/tools/support/build/makefile.start
release: force
	export SWHOME=$(SWHOME) &&  iros-rels -astd-onu-generic-rtk

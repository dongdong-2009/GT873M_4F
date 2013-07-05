
include makefile.rc

MAKEOPTS := SWHOME=$(SWHOME) \
            TOOLSHOME=$(TOOLSHOME) \
            BUILDID=$(BUILDID)

TARGET_LIST=iros libosal libplat libcache libmc libptp libflash libipintf libsys libtrmsvr liboam libweb libdb  libaal libswitch libsdl  libloopdetect libpppoe iros_onu
include $(SWHOME)/tools/support/build/makefile.start
release: force
	export SWHOME=$(SWHOME) &&  iros-rels -astd-onu-generic-rtk

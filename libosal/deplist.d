/mnt/hgfs/myhello/Cortina4FE_8032/libosal/osal_common.o: /mnt/hgfs/myhello/Cortina4FE_8016//src/cmn/plat/osal/osal_common.c
	echo "[ Compile $(CC) ]       $< "
	$(CC) $(CFLAGS) $(MOD_CFLAGS) $(MOD_INCLUDES) $(INCLUDES) $< -o $@
/mnt/hgfs/myhello/Cortina4FE_8032/libosal/osal_api_core.o: /mnt/hgfs/myhello/Cortina4FE_8016//src/cmn/plat/osal/ecos/osal_api_core.c
	echo "[ Compile $(CC) ]       $< "
	$(CC) $(CFLAGS) $(MOD_CFLAGS) $(MOD_INCLUDES) $(INCLUDES) $< -o $@
/mnt/hgfs/myhello/Cortina4FE_8032/libosal/osal_common.d: /mnt/hgfs/myhello/Cortina4FE_8016//src/cmn/plat/osal/osal_common.c
	[ -d $(MOD_TMPHOME) ] || clonedirs $(MOD_HOME) $(MOD_TMPHOME)
	$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(MOD_CFLAGS) $(MOD_INCLUDES) $(INCLUDES) $<  | sed '\''s!\($(addsuffix \.o, $(basename $(notdir $<)))\)[ :]*!$(@:.d=.o) $@ : !g '\'' | sed -e '1d' > $@ ; [ -s $@ ] || rm -f $@'
/mnt/hgfs/myhello/Cortina4FE_8032/libosal/osal_api_core.d: /mnt/hgfs/myhello/Cortina4FE_8016//src/cmn/plat/osal/ecos/osal_api_core.c
	[ -d $(MOD_TMPHOME) ] || clonedirs $(MOD_HOME) $(MOD_TMPHOME)
	$(SHELL) -ec '$(CC) -MM $(CFLAGS) $(MOD_CFLAGS) $(MOD_INCLUDES) $(INCLUDES) $<  | sed '\''s!\($(addsuffix \.o, $(basename $(notdir $<)))\)[ :]*!$(@:.d=.o) $@ : !g '\'' | sed -e '1d' > $@ ; [ -s $@ ] || rm -f $@'

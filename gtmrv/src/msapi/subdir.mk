
objs = gtBrgFdb.o \
gtBrgStp.o \
gtBrgStu.o \
gtBrgVlan.o \
gtBrgVtu.o \
gtCCPVT.o \
gtEvents.o \
gtPCSCtrl.o \
gtPhyCtrl.o \
gtPhyInt.o \
gtPIRL2.o \
gtPIRL.o \
gtPolicy.o \
gtPortCtrl.o \
gtPortPav.o \
gtPortRateCtrl.o \
gtPortRmon.o \
gtPortStat.o \
gtPortStatus.o \
gtPriTable.o \
gtPTP.o \
gtQosMap.o \
gtSysConfig.o \
gtSysCtrl.o \
gtSysStatus.o \
gtUtils.o \
gtVct.o \
gtVersion.o \
gtWeight.o \

OBJS += $(OBJ_DIR)/gtBrgFdb.o \
$(OBJ_DIR)/gtBrgStp.o \
$(OBJ_DIR)/gtBrgStu.o \
$(OBJ_DIR)/gtBrgVlan.o \
$(OBJ_DIR)/gtBrgVtu.o \
$(OBJ_DIR)/gtCCPVT.o \
$(OBJ_DIR)/gtEvents.o \
$(OBJ_DIR)/gtPCSCtrl.o \
$(OBJ_DIR)/gtPhyCtrl.o \
$(OBJ_DIR)/gtPhyInt.o \
$(OBJ_DIR)/gtPIRL2.o \
$(OBJ_DIR)/gtPIRL.o \
$(OBJ_DIR)/gtPolicy.o \
$(OBJ_DIR)/gtPortCtrl.o \
$(OBJ_DIR)/gtPortPav.o \
$(OBJ_DIR)/gtPortRateCtrl.o \
$(OBJ_DIR)/gtPortRmon.o \
$(OBJ_DIR)/gtPortStat.o \
$(OBJ_DIR)/gtPortStatus.o \
$(OBJ_DIR)/gtPriTable.o \
$(OBJ_DIR)/gtPTP.o \
$(OBJ_DIR)/gtQosMap.o \
$(OBJ_DIR)/gtSysConfig.o \
$(OBJ_DIR)/gtSysCtrl.o \
$(OBJ_DIR)/gtSysStatus.o \
$(OBJ_DIR)/gtUtils.o \
$(OBJ_DIR)/gtVct.o \
$(OBJ_DIR)/gtVersion.o \
$(OBJ_DIR)/gtWeight.o \


#all: 
#	$(AR) -r $(OBJ_DIR)/$(TARGET) $(OBJS)

#clean:
#	for obj in $(objs); do \
#		$(RM) -f $(OBJ_DIR)/$$obj; \
#	done	
		
$(OBJ_DIR)/%.o:	$(SRC_DIR)/msapi/%.c
	$(CC) $(CFLAGS) -c $< -o /$@ 

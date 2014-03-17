

OBJS += $(OBJ_DIR)/gtDrvConfig.o $(OBJ_DIR)/gtDrvEvents.o $(OBJ_DIR)/gtHwCntl.o

$(OBJ_DIR)/%.o:	$(SRC_DIR)/driver/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

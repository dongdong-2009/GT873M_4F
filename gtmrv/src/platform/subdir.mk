
objs = gtDebug.o \
gtMiiSmiIf.o \
gtSem.o \

OBJS += $(OBJ_DIR)/gtDebug.o \
$(OBJ_DIR)/gtMiiSmiIf.o \
$(OBJ_DIR)/gtSem.o \

$(OBJ_DIR)/%.o:	$(SRC_DIR)/platform/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

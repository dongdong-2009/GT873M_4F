
include Makefile.common

OBJS=

include $(SRC_DIR)/driver/subdir.mk
include $(SRC_DIR)/msapi/subdir.mk
include $(SRC_DIR)/platform/subdir.mk

TARGET = $(OBJ_DIR)/libgtmrv.a

export TOP_DIR	OBJS TARGET

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) -vru $@ $(OBJS)
	
clean: 
	$(RM) -f $(OBJ_DIR)/*.o	
	$(RM) -f $(TARGET)	
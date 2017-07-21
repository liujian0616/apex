DIRS	:= $(shell find . -maxdepth 3 -type d)
TARGET := channel_client_test
INCLUDE := -I../../../inc -I../../../inc/coroutine -I../../../inc/rpc -I../../../model
LIB := -L../../../lib -lapexbase -lpb64 -ldl
include ../../../makefile.inc
$(TARGET) : $(OBJS)
	$(XX) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIB)

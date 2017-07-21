DIRS	:= $(shell find . -maxdepth 3 -type d)
TARGET := net_server_test
INCLUDE := -I../../inc -I../../inc/coroutine -I../../inc/rpc
LIB := -L../../lib -lapexbase -lpb64 -ldl
include ../../makefile.inc
$(TARGET) : $(OBJS)
	$(XX) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIB)
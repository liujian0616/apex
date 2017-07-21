DIRS	:= $(shell find . -maxdepth 3 -type d)
TARGET := test
INCLUDE := -I../../inc -I ../../inc/hiredis
LIB := -L../../lib -lapexbase -lhiredis -lev

include ../../makefile.inc
$(TARGET) : $(OBJS)
	$(XX) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIB)

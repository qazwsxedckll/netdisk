SOUR:=$(wildcard *.c)
ELF:=a.out
OBJS:=$(SOUR:%.c=%.o)
CFLAGS:=$(shell mysql_config --cflags)
$(ELF):$(OBJS)
	gcc $^ -o $@ $(CFLAGS) -lmysqlclient -lpthread 
clean:
	rm -rf $(ELF) $(OBJS)

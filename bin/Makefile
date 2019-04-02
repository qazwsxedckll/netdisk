SOUR:=$(wildcard *.c)
ELF:=a.out
OBJS:=$(SOUR:%.c=%.o)
CFLAGS:=$(shell mysql_config --cflags)
$(ELF):$(OBJS)
	gcc $^ -o $@ $(CFLAGS) -lmysqlclient -lpthread 
	# gcc -g -o a.out *.c -I /usr/include/mysql -lmysqlclient -lpthread
clean:
	rm -rf $(ELF) $(OBJS)

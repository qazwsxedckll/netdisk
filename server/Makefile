SOUR:=$(wildcard bin/*.c)
ELF:=server
OBJS:=$(SOUR:%.c=%.o)
CFLAGS:=$(shell mysql_config --cflags)
$(ELF):$(OBJS)
	gcc $^ -o $@ $(CFLAGS) -lmysqlclient -lpthread -lcrypto 
	# gcc -g -o server bin/*.c -I /usr/include/mysql -lmysqlclient -lpthread -lcrypto
clean:
	rm -rf $(ELF) $(OBJS)

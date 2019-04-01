SOUR:=$(wildcard *.c)
ELF:=a.out
OBJS:=$(SOUR:%.c=%.o)
$(ELF):$(OBJS)
	# gcc $^ -o $@ -I /usr/include/mysql/ -lmysqlclient -lpthread
	gcc *.c -I /usr/include/mysql/ -lmysqlclient -lpthread
clean:
	rm -rf $(ELF) $(OBJS)


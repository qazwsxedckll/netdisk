SOUR:=$(wildcard *.c)
ELF:=main.exe
OBJS:=$(SOUR:%.c=%.o)
$(ELF):$(OBJS)
	# gcc $^ -o $@ -I /usr/include/mysql/ -lmysqlclient -lpthread
	gcc *.c -I /usr/include/mysql/ -lmysqlclient -lpthread
clean:
	rm -rf $(ELF) $(OBJS)


SOUR:=$(wildcard bin/*.c)
ELF:=client
OBJS:=$(SOUR:%.c=%.o)
$(ELF):$(OBJS)
	gcc $^ -o $@ -lpthread -lcrypto
clean:
	rm -rf $(ELF) $(OBJS)

CC = gcc
32BIT = -m32 -no-pie
GNU = -D_GNU_SOURCE
FLAGS = -Wall -Wextra -Wl,-Ttext-segment=0x20000000
LINKS = -ldl
SRCS = elph.c loder.c helper.c sym.c
OUT = elph
default: nodebug

nodebug: $(SRCS)
	$(CC) -DNODEBUG $(GNU) $(32BIT) $(NODBG) $(FLAGS) -o $(OUT) $(SRCS) $(LINKS)

debug: $(SRCS)
	$(CC) $(32BIT) $(GNU) $(FLAGS) -o $(OUT) $(SRCS) $(LINKS)

clean:
	rm $(OUT)

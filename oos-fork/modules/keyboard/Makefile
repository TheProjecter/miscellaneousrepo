-include ../../Makefile.cfg

FILES := $(shell find . -name "*.ooc")
NAME := $(shell pwd | awk -F/ '{print $$NF}')

.PHONY: all clean

all:
	${OOC} -c -v -g -sdk=../sdk -sourcepath=. -gc=off -cstrings -I../../include -$(CC) $(CCFLAGS) +-fpic +-fno-builtin -nolibcache -entrypoint=main -staticlib=../$(NAME).lib $(FILES)
	mkdir tmp && mv ../$(NAME).lib tmp && cd tmp && ar x $(NAME).lib && cd ..
	$(AS) $(ASFLAGS) -o tmp/info.o info.asm
	${LD} ${LDFLAGS} -soname $(NAME).mod -shared --allow-shlib-undefined -nostdlib -o ../$(NAME).mod `find tmp -name "*.o"`
	rm -rf tmp

clean:
	rm -rf ../$(NAME).lib ../$(NAME).mod tmp rock_tmp .libs

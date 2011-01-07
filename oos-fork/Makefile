-include Makefile.cfg

OOCFILES := $(shell find "src" -name "*.ooc")
ASMFILES := $(shell find "src" -name "*.asm")
ASMOBJFILES := $(patsubst %.asm,%.o,$(ASMFILES))

MODULES := $(shell find modules -mindepth 1 -maxdepth 1 -type d -regextype posix-extended -regex '^modules/(sd[^k]{1}|[^s]{1}dk|s[^d]{1}k|[^s]{1}[^d]{1}[^k]{1}|s[^d]{1}[^k]{1}|[^s]{1}[^d]{1}k|[^s]{1}d[^k]{1}|.{,2}|.{4,})$$')
MODFILES := $(addsuffix .mod,$(MODULES))
LIBFILES := $(addprefix modules/,$(addsuffix .lib,$(MODULES)))		

.PHONY: all clean bochs bochs-dbg

all: bochs

qemu: floppy
	qemu -fda floppy.img

bochs: floppy
	bochs -qf .bochsrc

floppy: oos.exe $(addsuffix .mod_build,$(MODULES))
	sudo mount -o loop floppy.img mnt
	sudo rm -rf mnt/system/*
	sudo mkdir mnt/system/modules
	sudo cp -r $(MODFILES) mnt/system/modules
	sudo cp -r oos.exe mnt/system
	sudo umount mnt

bochs-dbg: all
	bochs -qf .bochsrc-dbg

oos.exe: ${ASMOBJFILES} src/oos.lib
	${LD} ${LDFLAGS} -T src/linker.ld -o $@ ${ASMOBJFILES} src/oos.lib

src/oos.lib: ${OOCFILES}
	${OOC} ${OOCFLAGS} -nolibcache -entrypoint=kmain -staticlib=$@ boot/main.ooc

%.mod_build:
	make -C $*

%.mod_clean:
	make -C $* clean

%.o: %.asm
	$(AS) ${ASFLAGS} -o $@ $<

clean: $(addsuffix .mod_clean,$(MODULES))
	 $(RM) -r src/oos.lib oos.exe oos.iso tmp rock_tmp ooc_tmp .libs

#bochscd: all
#       bochs -qf .bochsrc

#oos.iso: oos.exe isofs/boot/grub/stage2_eltorito isofs/boot/grub.cfg
#       @mkdir -p isofs/system
#       cp $< isofs/system
#       genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -input-charset utf-8 -o $@ isofs
#       rm -rf isofs/boot/grub

#isofs/boot/grub/stage2_eltorito:
#       mkdir -p isofs/boot/grub
#       grub-mkimage -p /boot -o isofs/boot/grub/core.img biosdisk iso9660 multiboot sh
#       cat /usr/lib/grub/i386-pc/cdboot.img isofs/boot/grub/core.img > ${STAGE2}

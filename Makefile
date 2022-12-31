BUILD:=./build
SRC:=.

ENTRYPOINT:=0x10000

CFLAGS:= -m32 # 32 位的程序
CFLAGS+= -fno-builtin	# 不需要 gcc 内置函数
CFLAGS+= -nostdinc		# 不需要标准头文件
CFLAGS+= -fno-pic		# 不需要位置无关的代码  position independent code
CFLAGS+= -fno-pie		# 不需要位置无关的可执行程序 position independent executable
CFLAGS+= -nostdlib		# 不需要标准库
CFLAGS+= -fno-stack-protector	# 不需要栈保护
CFLAGS:=$(strip ${CFLAGS})

DEBUG:= -g
INCLUDE:=-I$(SRC)/include

image:
	bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat master.img

$(BUILD)/boot/%.bin: $(SRC)/boot/%.s
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(BUILD)/%.o: $(SRC)/%.c
	$(shell mkdir -p $(dir $@))
	gcc $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@

$(BUILD)/%.o: $(SRC)/%.s
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 -gdwarf $< -o $@

$(BUILD)/kernel.bin: $(BUILD)/kernel/start.o \
	$(BUILD)/kernel/main.o \
	$(BUILD)/kernel/start.o \
	$(BUILD)/kernel/io.o \
	$(BUILD)/lib/string.o \
	$(BUILD)/kernel/console.o\
	$(BUILD)/kernel/printk.o \
	$(BUILD)/lib/vsprintf.o \
	$(BUILD)/kernel/assert.o \
	$(BUILD)/kernel/debug.o \
	$(BUILD)/kernel/global.o \
	$(BUILD)/kernel/schedule.o \
	$(BUILD)/kernel/task.o \
	$(BUILD)/kernel/interrupt.o \
	$(BUILD)/kernel/handler.o \
	$(BUILD)/lib/stdlib.o \
	$(BUILD)/kernel/clock.o \
	$(BUILD)/kernel/time.o \
	$(BUILD)/kernel/rtc.o \
	$(BUILD)/kernel/memory.o \
	$(BUILD)/kernel/bitmap.o \
	$(BUILD)/kernel/gate.o \
	$(BUILD)/lib/syscall.o \
	$(BUILD)/lib/list.o \
	$(BUILD)/kernel/thread.o \
	$(BUILD)/kernel/mutex.o

	$(shell mkdir -p $(dir $@))
	ld -m elf_i386 -static $^ -o $@ -Ttext $(ENTRYPOINT)

$(BUILD)/system.bin: $(BUILD)/kernel.bin
	objcopy -O binary $< $@

$(BUILD)/system.map: $(BUILD)/kernel.bin
	nm $< | sort > $@

$(BUILD)/master.img: $(BUILD)/boot/boot.bin \
	$(BUILD)/boot/loader.bin \
	$(BUILD)/system.bin \
	$(BUILD)/system.map \

	dd if=$(BUILD)/boot/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BUILD)/boot/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD)/system.bin of=$@ bs=512 count=200 seek=10 conv=notrunc

test:$(BUILD)/master.img

.PHONY:clean
clean:
	rm -rf $(BUILD)/boot/*
	rm -rf $(BUILD)/kernel/*
	rm -rf $(BUILD)/lib/*
	rm $(BUILD)/system.map
	rm $(BUILD)/system.bin
	rm $(BUILD)/kernel.bin

.PHONY: bochs
	bochs: $(BUILD)/master.img
			bochs -q

QEMU :=qemu-system-i386 \
	-m 32M \
	-boot c \
	-drive file=$(BUILD)/master.img,index=0,media=disk,format=raw \
	-audiodev pa,id=hda \
	-rtc base=localtime \
	-machine pcspk-audiodev=hda \

.PHONY: qemu
qemu: $(BUILD)/master.img
	$(QEMU)

.PHONY: qemug
qemug: $(BUILD)/master.img
	$(QEMU) -s -S 

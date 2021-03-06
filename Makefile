
GPPPARAMS = -m32 -Iinclude -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore
ASPARAMS = --32
LDPARAMS = -melf_i386

objects = obj/loader.o \
		  obj/kernel.o \
		  obj/gdt.o \
		  obj/memorymanagement.o \
		  obj/drivers/driver.o \
		  obj/hardwarecommunication/port.o \
		  obj/hardwarecommunication/interrupts.o \
		  obj/hardwarecommunication/pci.o \
		  obj/hardwarecommunication/interruptstubs.o \
		  obj/syscalls.o \
		  obj/multitasking.o \
		  obj/net/etherframe.o \
		  obj/net/arp.o \
		  obj/net/ipv4.o \
		  obj/net/icmp.o \
		  obj/net/udp.o \
		  obj/net/tcp.o \
		  obj/drivers/amd_am79c973.o \
		  obj/drivers/keyboard.o \
		  obj/gui/widget.o \
		  obj/gui/window.o \
		  obj/gui/desktop.o \
		  obj/drivers/vga.o \
		  obj/drivers/ata.o\
		  obj/drivers/mouse.o

run: mykernel.iso
	(killall virtualboxvm && sleep 1) || true
	virtualboxvm --startvm "my os" &

obj/%.o: src/%.cpp
	mkdir -p $(@D)
	# g++ $(GPPPARAMS) -o $@ -c $<
	gcc $(GPPPARAMS) -c -o $@ $<

obj/%.o: src/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

mykernel.bin: linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o $@ $(objects)

mykernel.iso: mykernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp mykernel.bin iso/boot/mykernel.bin
	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo '' >> iso/boot/grub/grub.cfg
	echo 'menuentry "my os" {' >> iso/boot/grub/grub.cfg
	echo '	multiboot /boot/mykernel.bin' >> iso/boot/grub/grub.cfg
	echo '	boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=$@ iso
	rm -rf iso

install: mykernel.bin
	sudo cp $< /boot/mykernel.bin

.PHONY: clean
clean:
	rm -rf obj mykernel.bin mykernel.iso
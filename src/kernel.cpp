#include <gdt.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/driver.h>
#include <common/types.h>

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

void printf(const char* str){
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x = 0, y = 0;
    for(int i = 0; str[i] != '\0'; i++) {
    switch(str[i]) {
        case '\n' :
            y++;
            x = 0;
            break;
        default:
            VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | str[i];
            x++;
            break;
    }
        if(x >= 80) {
            x = 0;
            y++;
        }
        if(y >= 25) {
            for(y = 0; y < 25; y++){
                for(x = 0; x < 80; x++){
                    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | ' ';
                }
            }
            y = 0;
            x = 0;
        }    
    }
        
}

void printfHex(uint8_t key) {
    char* foo = (char *)"00";
    const char* hex = "0123456789ABCDEF";
    foo[11] = hex[(key >> 4) & 0x0F];
    foo[12] = hex[key & 0x0F];
    printf((const char*)foo);       
}

class PrintfKeyboardEventHandler : public KeyboardEventHandler {
public:
    void OnKeyDown(char c) {
        char* foo = (char*) " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler {
    int8_t x, y;
public:
    MouseToConsole() {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40, y = 12;
        VideoMemory[80 * y + x] = ((VideoMemory[80 * y + x] & 0xF000) >> 4)
                                | ((VideoMemory[80 * y + x] & 0x0F00) << 4)
                                | ((VideoMemory[80 * y + x] & 0x00FF));
    }

    void OnMouseMove(int xoffset, int yoffset) {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80 * y + x] = ((VideoMemory[80 * y + x] & 0xF000) >> 4)
                                | ((VideoMemory[80 * y + x] & 0x0F00) << 4)
                                | ((VideoMemory[80 * y + x] & 0x00FF));
        x += xoffset;
        if(x < 0) x = 0;
        if(x >= 80) x = 79;

        y += yoffset;
        if(y < 0)   y = 0;
        if(y >= 25) y = 24;
        
        VideoMemory[80 * y + x] = ((VideoMemory[80 * y + x] & 0xF000) >> 4)
                                | ((VideoMemory[80 * y + x] & 0x0F00) << 4)
                                | ((VideoMemory[80 * y + x] & 0x00FF));
    }
};

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;

extern "C" void callConstructors() {
    for (constructor* i = &start_ctors; i != &end_ctors; i++) {
        (*i)();
    }
} 

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber) {
    printf("hello ziqiOS!\n");
    printf("hello inori!\n");
    printf("hello world!\n");
    GlobalDescriptorTable gdt;
    InterruptManager interrupts(0x20, &gdt);

    printf("Initializing Hardware, Stage 1\n");

    DriverManager drvManager;

        PrintfKeyboardEventHandler kbhanler;
        KeyboardDriver keyboard(&interrupts, &kbhanler);
        drvManager.AddDriver(&keyboard);

        MouseToConsole mousehandler;
        MouseDriver mouse(&interrupts, &mousehandler);
        drvManager.AddDriver(&mouse);

    printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();

    printf("Initializing Hardware, Stage 3\n");
    interrupts.Activate();
    while(1);
}
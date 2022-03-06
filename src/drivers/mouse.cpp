#include <drivers/mouse.h>

using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

void printf(const char*);

MouseEventHandler::MouseEventHandler() {

}

void MouseEventHandler::OnActivate() {

}

void MouseEventHandler::OnMouseDown(uint8_t button) {

}

void MouseEventHandler::OnMouseUp(uint8_t button) {
}

void MouseEventHandler::OnMouseMove(int x, int y) {

}



MouseDriver::MouseDriver(InterruptManager* manager, MouseEventHandler* handler)
    : InterruptHandler(0x2C, manager),
    dataport(0x60), 
    commandport(0x64) {
    
    this->handler = handler;

}

MouseDriver::~MouseDriver() {

}

void MouseDriver::Activate() {
    offset = 0;
    buttons = 0;
    commandport.write(0xA8); //activate interrupts
    commandport.write(0x20); //get current state
    uint8_t status = dataport.read() | 2;
    commandport.write(0x60); //set state
    dataport.write(status);

    commandport.write(0xD4);
    dataport.write(0xF4);
    dataport.read();
}

uint32_t MouseDriver::HandleInterrupt(uint32_t esp) {
    uint8_t status = commandport.read();
    if(!(status & 0x20))
        return esp;
    //static int8_t x = 40, y = 12;
    buffer[offset] = dataport.read();
    
    if(handler == 0)
        return esp;

    offset = (offset + 1) % 3;
    if(offset == 0) {
        if(buffer[1] != 0 || buffer[2] != 0) {
            handler->OnMouseMove(buffer[1], -buffer[2]);
        }
        
        for(uint8_t i = 0; i < 3; i++) {
            if((buffer[0] & (0x01 << i)) != (buttons & (0x01 << i))) {
                if(buttons & (0x1<<i))
                    handler->OnMouseUp(i + 1);
                else
                    handler->OnMouseDown(i + 1);
            }
        }
        buttons = buffer[0];
        
    }
    return esp;
}

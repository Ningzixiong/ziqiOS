#include <drivers/ata.h>

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;

void printf(const char*);
void printfHex(uint8_t);

AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(common::uint16_t portBase, bool master) 
    :dataPort(portBase),
    errorPort(portBase + 1),
    sectorCountPort(portBase + 2),
    lbaLowPort(portBase + 3),
    lbaMidPort(portBase + 4),
    lbaHiPort(portBase + 5),
    devicePort(portBase + 6),
    commandPort(portBase + 7),
    controlPort(portBase + 0x206) {

    bytesPerSector = 512;
    this->master = master;
}
AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment() {}

void AdvancedTechnologyAttachment::Identify() {
    devicePort.write(master ? 0xA0 : 0xB0);
    controlPort.write(0);

    devicePort.write(0xA0);
    uint8_t status = commandPort.read();
    if(status == 0xFF)
        return ;
    devicePort.write(master ? 0xA0 : 0xB0);
    sectorCountPort.write(0);
    lbaLowPort.write(0);
    lbaMidPort.write(0);
    lbaHiPort.write(0);
    commandPort.write(0xEC);

    status = commandPort.read();
    if(status == 0x00) 
        return ; // no device;
    
    while(((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01)) {
            status = commandPort.read();
        }
    
    if(status & 0x01) {
        printf("ERROR");
        return ;
    }

    for(uint16_t i = 0; i < 256; i++) {
        uint16_t data = dataPort.read();
        char* foo = " \0";
        foo[1] = (data >> 8) & 0x00FF;
        foo[0] = data & 0x00FF;
        printf(foo);
    }

}
void AdvancedTechnologyAttachment::Read28(common::uint32_t sector, common::uint8_t* data, int count) {
    if(sector & 0xF0000000)
        return ;
    if(count > bytesPerSector)
        return ;

    devicePort.write((master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    errorPort.write(0);
    sectorCountPort.write(1);

    lbaLowPort.write( sector & 0x000000FF );
    lbaMidPort.write((sector & 0x0000FF00) >> 8);
    lbaHiPort.write( (sector & 0x00FF0000) >> 16);
    commandPort.write(0x20);

    uint8_t status = commandPort.read();
    if(status == 0x00) 
        return ; // no device;
    
    while(((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01)) {
            status = commandPort.read();
        }
    
    if(status & 0x01) {
        printf("ERROR");
        return ;
    }

    printf("Reading from ATA: ");

    for(uint16_t i = 0; i < count; i += 2) {
        uint16_t wdata = dataPort.read();
        
        char* foo = " \0";
        foo[1] = (wdata >> 8) & 0x00FF;
        foo[0] = wdata & 0x00FF;
        printf(foo);

        data[i] = wdata & 0x00FF;
        if(i + 1 < count) 
            data[i + 1] = (wdata >> 8) & 0x00FF;
    }

    for(uint16_t i = count + (count % 2); i < bytesPerSector; i += 2) {
        dataPort.read();
    }
}


void AdvancedTechnologyAttachment::Write28(common::uint32_t sector, common::uint8_t* data, int count) {
    if(sector & 0xF0000000)
        return ;
    if(count > bytesPerSector)
        return ;

    devicePort.write((master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    errorPort.write(0);
    sectorCountPort.write(1);

    lbaLowPort.write( sector & 0x000000FF );
    lbaMidPort.write((sector & 0x0000FF00) >> 8);
    lbaHiPort.write( (sector & 0x00FF0000) >> 16);
    commandPort.write(0x30);

    printf("Writing to ATA: ");

    for(uint16_t i = 0; i < count; i += 2) {
        uint16_t wdata = data[i];
        if(i+1 < count)
            wdata |= ((uint16_t)data[i + 1] << 8);
        
        char* foo = " \0";
        foo[1] = (wdata >> 8) & 0x00FF;
        foo[0] = wdata & 0x00FF;
        printf(foo);

        dataPort.write(wdata);
    }

    for(uint16_t i = count + (count % 2); i < bytesPerSector; i += 2) {
        dataPort.write(0x0000);
    }

}


void AdvancedTechnologyAttachment::Flush() {
    devicePort.write(master ? 0xE0 : 0xF0);
    commandPort.write(0xE7);

    uint8_t status = commandPort.read();
    while(((status & 0x80) == 0x80)
        && ((status & 0x01) != 0x01)) {
            status = commandPort.read();
        }
    
    if(status & 0x01) {
        printf("ERROR");
        return ;
    }
}



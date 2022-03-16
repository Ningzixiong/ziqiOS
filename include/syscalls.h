#ifndef __MYOS__SYSCALLS_H
#define __MYOS__SYSCALLS_H

#include <common/types.h>
#include <multitasking.h>
#include <hardwarecommunication/interrupts.h> 

namespace myos {
    class SyscallHandler : public hardwarecommunication::InterruptHandler {
    public:
        SyscallHandler(myos::common::uint8_t interruptNumber, hardwarecommunication::InterruptManager* interruptManager);
        ~SyscallHandler();

        virtual myos::common::uint32_t HandleInterrupt(myos::common::uint32_t esp);
    };
}

#endif


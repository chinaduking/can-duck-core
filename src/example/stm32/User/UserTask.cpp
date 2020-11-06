#include "UserTask.h"
#include "Bsp.hpp"
#include "string.h"

char* msg = (char*)"hello world!\n\r";
mcu_driver::Usart* p_serial;

void BootTaskAfterHwInit(){
	BspInit();

	led[0].set(1);
	led[1].set(0);

	p_serial = &hostSerial;
}

int cnt = 0;

void MainLoopTask(){
	if(!p_serial->isWriteBusy()){
		p_serial->write((uint8_t*)msg, strlen(msg)+1);

		cnt ++;
	}
}

void SysTickTask(){

}

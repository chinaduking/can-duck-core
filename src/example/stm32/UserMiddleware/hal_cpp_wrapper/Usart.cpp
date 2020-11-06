/*
 * Usart.cpp
 *
 *  Created on: Oct 11, 2020
 *      Author: sdong
 */

#include "Usart.hpp"

using namespace mcu_driver;

Usart::Usart(UART_HandleTypeDef* hal_handle, Mode mode):
		hal_handle(hal_handle), mode(mode){
}

bool Usart::isWriteBusy(){
	return is_busy;
}

int32_t Usart::read(uint8_t *data, uint32_t len){
	return 0;
}


int32_t Usart::write(const uint8_t *data, uint32_t len){
	switch(mode){
		case Mode::Blocking:
			send_data = (uint8_t*)data;
			send_len = len;
			is_busy = true;
			HAL_UART_Transmit(hal_handle, send_data, send_len, 1000);
			is_busy = false;
		break;

		case Mode::Dma:
			send_data = (uint8_t*)data;
			send_len = len;
			HAL_UART_Transmit_DMA(hal_handle, send_data, send_len);
		break;

		case Mode::SwMan:
			break;

		case Mode::Interrupt:
			break;

		default: break;
	}

	return len;
}

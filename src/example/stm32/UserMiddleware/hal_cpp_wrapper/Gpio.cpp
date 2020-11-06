/*
 * Gpio.cpp
 *
 *  Created on: Oct 15, 2020
 *      Author: sdong
 */
#include "Gpio.hpp"

using namespace mcu_driver;

Gpio::Gpio(GPIO_TypeDef* port, uint16_t pin):
		port(port),pin(pin){}


void Gpio::set(uint8_t val){
	HAL_GPIO_WritePin(port, pin, (GPIO_PinState)val);
}





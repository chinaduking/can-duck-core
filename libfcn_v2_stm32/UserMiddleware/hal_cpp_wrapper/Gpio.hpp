/*
 * Gpio.hpp
 *
 *  Created on: Oct 12, 2020
 *      Author: sdong
 */

#ifndef MCUDRIVER_GPIO_HPP
#define MCUDRIVER_GPIO_HPP

#include "stm32f4xx_hal.h"

namespace mcu_driver{
	class Gpio{
	public:
		Gpio(GPIO_TypeDef* port, uint16_t pin);
		void set(uint8_t val);
		uint8_t get();

	private:
		GPIO_TypeDef* const port;

		const uint16_t pin;
	};
}




#endif /* MCUDRIVER_GPIO_HPP */

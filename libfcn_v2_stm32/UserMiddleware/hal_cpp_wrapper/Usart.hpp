#ifndef MCUDRIVER_USART_HPP
#define MCUDRIVER_USART_HPP

#include "stm32f4xx_hal.h"
#include "LLComDevice.hpp"

namespace mcu_driver{

	class Usart : public LLByteDevice{
	public:
		enum class Mode{
			Blocking,
			Dma,
			SwMan,
			Interrupt
		};

		/*
		 * mode:
		 * 	0. blocking send & recv
		 * 	1. DMA send send & recv
		 * 	2. main loop polling send & recv
		 * 	3. Interrupt send & recv
		 * */
		Usart(UART_HandleTypeDef* hal_handle, Mode mode);
		~Usart() = default;

		bool isWriteBusy();// override;
		int32_t read(uint8_t *data, uint32_t len) override;
	    int32_t write(const uint8_t *data, uint32_t len) override;


//		void write(uint8_t* data, uint32_t len);
//		uint32_t read(uint8_t* data, uint32_t len);
	private:
		UART_HandleTypeDef* const hal_handle;
		const Mode mode;
		uint8_t* send_data;
		uint32_t send_len;
		bool is_busy{false};
	};

}

#endif //MCUDRIVER_USART_HPP


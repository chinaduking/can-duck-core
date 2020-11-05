/*
 * Can.hpp
 *
 *  Created on: Oct 11, 2020
 *      Author: sdong
 */

#ifndef MCUDRIVER_CAN_HPP
#define MCUDRIVER_CAN_HPP

#include "stm32f4xx_hal.h"
#include "LLComDevice.hpp"

namespace mcu_driver{

	class Can : public LLCanBus{
	public:
		/*
		 * mode:
		 * 	0. blocking send & recv
		 * 	1. DMA send send & recv
		 * 	2. main loop polling send & recv
		 * 	3. Interrupt send & recv
		 * */
		Can(CAN_HandleTypeDef* hal_handle, uint8_t mode);
		~Can() = default;

//		bool isWriteBusy() override;
	    int32_t read(Frame* frame) override;
	    int32_t write(Frame* frame) override;


//		void write(uint8_t* data, uint32_t len);
//		uint32_t read(uint8_t* data, uint32_t len);
	private:
	    CAN_HandleTypeDef* const hal_handle;
		uint8_t* send_data;
		uint32_t send_len;
	};

}



#endif //MCUDRIVER_CAN_HPP

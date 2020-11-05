#include "Bsp.hpp"

extern UART_HandleTypeDef huart1;
mcu_driver::Usart hostSerial(&huart1, mcu_driver::Usart::Mode::Blocking);

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

//mcu_driver::Can mainCan(&hcan1, 0);

mcu_driver::Gpio led[3] = {
		mcu_driver::Gpio(GPIOE, GPIO_PIN_3),
		mcu_driver::Gpio(GPIOE, GPIO_PIN_4),
		mcu_driver::Gpio(GPIOG, GPIO_PIN_9)
};

void BspInit(){


}

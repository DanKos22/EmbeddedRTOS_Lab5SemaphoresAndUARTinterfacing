/*
 * userApp.c
 *
 *  Created on: Dec 8, 2023
 *      Author: Niall.OKeeffe@atu.ie
 */

#include "userApp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

//--------------------------------------------------------------
//used for real time stats, do not delete code from this section
extern TIM_HandleTypeDef htim7;
extern volatile unsigned long ulHighFrequencyTimerTicks;
void configureTimerForRunTimeStats(void)
{
    ulHighFrequencyTimerTicks = 0;
    HAL_TIM_Base_Start_IT(&htim7);
}
unsigned long getRunTimeCounterValue(void)
{
	return ulHighFrequencyTimerTicks;
}
//end of real time stats code
//----------------------------------------------------------------

extern UART_HandleTypeDef huart1;


static void buttonTask(void * pvParameters);
static void countdownTask(void * pvParameters);
//static void timerTask(void * pvParameters);
SemaphoreHandle_t buttonSemaphore = NULL;
SemaphoreHandle_t timerSemaphore = NULL;

// _write function used for printf
int _write(int file, char *ptr, int len) {
	HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
	return len;
}


void userApp() {
	printf("Starting application\r\n\n");

	//HAL_UART_Receive_IT(&huart1, &ch, 1);
	//HAL_TIM_Base_Start_IT(&htim6);

	xTaskCreate(buttonTask, "Button Task", 200, NULL, 1, NULL);
	xTaskCreate(countdownTask, "Display Task", 200, NULL, 3, NULL);
	//xTaskCreate(timerTask, "Timer Task", 200, NULL, 2, NULL);
	buttonSemaphore = xSemaphoreCreateBinary();
	timerSemaphore = xSemaphoreCreateBinary();
	vQueueAddToRegistry(buttonSemaphore, "Button Semaphore");
	vQueueAddToRegistry(timerSemaphore, "Timer Semaphore");
	vTaskStartScheduler();

	while(1) {
	}
}



void buttonTask(void * pvParameters) {
	printf("Starting button task \r\n\n");
	while(1) {
		//wait for switch press
		if(HAL_GPIO_ReadPin(BUTTON_EXTI13_GPIO_Port, BUTTON_EXTI13_Pin)== 0){
			printf("Switch press detected, giving semaphore\r\n\n");
			//Give Semaphore
			xSemaphoreGive(buttonSemaphore);

			//wait for switch release
			while(HAL_GPIO_ReadPin(BUTTON_EXTI13_GPIO_Port, BUTTON_EXTI13_Pin)== 0);
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void countdownTask(void * pvParameters) {
	uint8_t sec = 30;
	printf("Starting Display Task \r\n\n");

	while(1) {
		if(xSemaphoreTake(buttonSemaphore, 0) == pdTRUE){
			printf("Semaphore taken, Resetting timer...\r\n\n");
			sec = 30;
			//toggle LED2
			//HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		}
		printf("Time: %d\r\n\n", sec);

		if(sec > 0){
			sec--;
		}
		if(sec <= 10){
			HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		}
		vTaskDelay(pdMS_TO_TICKS(300));
	}
}

/*
void timerTask(void * pvParameters) {
	printf("Starting timer task \r\n\n");
	while(1) {
		vTaskDelay(pdMS_TO_TICKS(100));
	    xSemaphoreGive(timerSemaphore);
	}

}*/



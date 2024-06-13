/*
 * Timemanagement_us.h
 *
 *  Created on: 20.12.2018
 *      Author: schroeder
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TIMEMANAGEMENT_H_
#define TIMEMANAGEMENT_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"
#include "stdbool.h"


 void CalculateTime(void);
 void delayUS_DWT(uint32_t us);
 void tic(void);
 float toc(void);

 extern uint32_t TimeSinceStartup32; /*In ms, Returned value from HAL_GetTick(), reset after 49 days*/
 extern uint32_t TimeSinceStartup32Old; /*In ms, used to test if this time is reset*/
 extern uint64_t TimeSinceStartup64; /*In ms, never reset, use this as time stamp in program*/
 extern uint64_t TimeSinceStartup64Old;
 extern uint64_t Cycles;
 extern uint64_t CycleTimeAverage; // in us
 extern uint64_t LastTimeSinceStartup64;
 extern uint64_t CycleTimeFailed150ms;
 extern uint64_t StandardTimerNextPoint; /*100ms Timer*/
 extern uint64_t FastTimerNextPoint; /*4ms Timer*/
 extern uint64_t SlowTimerNextPoint; /*1000ms Timer*/
 extern uint64_t VerySlowTimerNextPoint; /*1 hour Timer*/
 extern uint64_t DisplayControlTimerNextPoint;

 typedef struct Timer {
	 uint32_t period;
	 uint64_t nextTrigger;
	 uint64_t* input;
	 bool Q;
 } Timer;

 void timer_init(Timer* timer, uint32_t period, uint64_t* input);
 bool timer_run(Timer* timer);


#ifdef __cplusplus
 }
#endif

#endif /* TIMEMANAGEMENT_H_ */

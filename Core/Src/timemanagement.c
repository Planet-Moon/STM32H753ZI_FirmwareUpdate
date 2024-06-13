#include "timemanagement.h"

 uint32_t TimeSinceStartup32 = 0; /*In ms, Returned value from HAL_GetTick(), reset after 49 days*/
 uint32_t TimeSinceStartup32Old = 0; /*In ms, used to test if this time is reset*/
 uint64_t TimeSinceStartup64 = 0; /*In ms, never reset, use this as time stamp in program*/
 uint64_t TimeSinceStartup64Old = 0;
 uint64_t Cycles = 0;
 uint64_t CycleTimeAverage = 0; // in us
 uint64_t LastTimeSinceStartup64 = 0;
 uint64_t CycleTimeFailed150ms = 0;
 uint64_t StandardTimerNextPoint = 0; /*100ms Timer*/
 uint64_t FastTimerNextPoint = 0; /*4ms Timer*/
 uint64_t SlowTimerNextPoint = 0; /*1000ms Timer*/
 uint64_t VerySlowTimerNextPoint = 3600000; /*1 hour Timer*/
 uint64_t DisplayControlTimerNextPoint = 0;

 void CalculateTime(void)
 {
 	TimeSinceStartup32 = HAL_GetTick();
 	if(TimeSinceStartup32 < TimeSinceStartup32Old)
 	{
 		TimeSinceStartup64 += (0x0000000100000000 + TimeSinceStartup32 - TimeSinceStartup32Old);
 	}
 	else
 	{
 		TimeSinceStartup64 += (TimeSinceStartup32 - TimeSinceStartup32Old);
 	}
 	TimeSinceStartup32Old = TimeSinceStartup32;
 }


/** \brief: Wait time in microseconds
 *	\param us: time to be waited in microseconds
 */
#pragma GCC push_options
#pragma GCC optimize ("O3")
void delayUS_DWT(uint32_t us) {
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;	//aktivate DWT-Unit
	DWT->CTRL |= 1; // enable the counter
//	DWT->CYCCNT = 0; // reset the counter
	volatile uint32_t factor = (SystemCoreClock/1000000L);
	volatile uint32_t cycles = factor*us;

	volatile uint32_t start = DWT->CYCCNT;
	do  {
	} while((DWT->CYCCNT - start) < cycles);
}
#pragma GCC pop_options

/** \brief: Starts the time measurement
 */
#pragma GCC push_options
#pragma GCC optimize ("O3")
void tic(void){
//	__HAL_TIM_SET_COUNTER(&htim1, 0);	//reset counter if Timer1 to 0
//	HAL_TIM_Base_Start_IT(&htim1);			//start Timer1
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;	//aktivate DWT-Unit
	DWT->CTRL |= 1;	//cyccounter enable
	DWT->CYCCNT = 0;	//cyccoutner clear
}
#pragma GCC pop_options

/** \brief: Ends the time measurement
 *  \return: Elapsed time since toc in milliseconds
 */
#pragma GCC push_options
#pragma GCC optimize ("O3")
float toc(void){	//better: inline or as macro
	volatile uint32_t counter_per_1ms = 96000;
	volatile uint32_t factor = (SystemCoreClock/1000000L);
	volatile uint32_t counter_value = DWT->CYCCNT; //cyccounter read
	volatile float dividend = counter_value;
	volatile float divisor = counter_per_1ms*factor;
	float ret_val = dividend*100/divisor;
	return	ret_val;
}
#pragma GCC pop_options

void timer_init(Timer* timer, uint32_t period, uint64_t* input){
	timer->period = period;
	timer->input = input;
	timer->nextTrigger = (*timer->input) + timer->period;
}

bool timer_run(Timer* timer){
	timer->Q = (*timer->input) >= timer->nextTrigger;
	if(timer->Q)
		timer->nextTrigger = (*timer->input) + timer->period;
	return timer->Q;
}

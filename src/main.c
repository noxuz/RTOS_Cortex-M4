/*
 * Abraham Rodriguez Vazquez
 * Round Robin RTOS for the ARM Cortex-M4 microntroller
 *
 * Description: The 4 threads are added to the scheduler and context-switch
 * is performed at each interrupt of the SysTick.
 */

#include "S32K142.h"
#include <stdint.h>

#include "osKernel.h"

#define QUANTA   (1u) /* 10 milisegundos */

uint32_t count0, count1, count2, count3;
#define INTCTRL (*((volatile uint32_t*) 0xE000ED04))


void task0(void)
{
    while(1)
    {
        count0++;
        
        // Example for yielding the thread, i.e. pending a systick interrupt
        //S32_SCB->ICSR |= 1<<26; 
    }
}

void task1(void)
{
    
    // Exampe for yielding the thread
    while(1)
    {
        count1++;
     
        // Restart value of systick and pend a systick interrupt for yielding the thread
        S32_SysTick->CVR = 0;
        INTCTRL = 0x04000000;


    }
}

// Thread with a simple incrementing variable
void task2(void)
{
    while(1)
    {
        count2++;

    }
}

// Thread with a simple incrementing variable
void task3(void)
{
    while(1)
    {
        count3++;

    }
}

int main(void)
{

    /* Add the 4 threads to the kernel */
    osKernelAddThreads(task0, task1, task2,task3);

    /* Setup and launch of the OS */
    osKernelLaunch(QUANTA);
    osSchedulerLaunch();

}

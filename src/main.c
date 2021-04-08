/*
 * main implementation: use this 'C' sample to create your own application
 *
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
        //S32_SCB->ICSR |= 1<<26;
    }
}

void task1(void)
{
    while(1)
    {count1++;
        S32_SysTick->CVR = 0;
        INTCTRL = 0x04000000;


    }
}
void task2(void)
{
    while(1)
    {
        count2++;

    }
}

void task3(void)
{
    while(1)
    {
        count3++;

    }
}

int main(void)
{
/* añadir threads */

    osKernelAddThreads(task0, task1, task2,task3);

    /* Arrancar */
    osKernelLaunch(QUANTA);
    osSchedulerLaunch();

}

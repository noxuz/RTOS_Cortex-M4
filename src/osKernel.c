
#include "s32_core_cm4.h"
#include "s32k142.h"
#include "osKernel.h"


/* Parametros */
#define NUM_THREADS     (4u)
#define STACK_SIZE      (100u)

#define CLOCK_FREQ      (48000000)

#define SYSPRI2 (*((volatile uint32_t*) 0xE000ED20))


/* Thread control block */
typedef struct tcb{
    uint32_t* stackPtr;
    struct tcb* nextPtr;
} TCB_type;

/* Linked list */
TCB_type tcbs[NUM_THREADS];
TCB_type* currentPtr;

/* Stack */
uint32_t TCB_Stack[NUM_THREADS][STACK_SIZE];


/* Funcion para inicializar el stack pointer de los threads */
void osKernelStackInit(uint32_t thread_num)
{
    /* eL STACK POINTER INICIAL de cada thread es el pointer del top of stack de su stack */
    tcbs[thread_num].stackPtr = &TCB_Stack[thread_num][STACK_SIZE-16];

    /* Poner en thumb mode activando el PSR */
    TCB_Stack[thread_num][STACK_SIZE-1] = 0x01000000;
}

void osKernelLaunch(uint32_t quanta)
{
    DISABLE_INTERRUPTS()

    S32_SysTick->CSR &= ~S32_SysTick_CSR_ENABLE_MASK; /* Apagar Systick */
    S32_SysTick->RVR = ((CLOCK_FREQ/1000) * quanta) - 1; /* Cargar valor para obtener quanta en milisegundos */

    /* Core clock como fuente y prender interrupcion */
    S32_SysTick->CSR |=  S32_SysTick_CSR_CLKSOURCE(1) | S32_SysTick_CSR_TICKINT(1);

    /* Light it up */
    S32_SysTick->CSR |= S32_SysTick_CSR_ENABLE_MASK;
    ENABLE_INTERRUPTS()

    //S32_SysTick->CSR = 0;
    //S32_SysTick->CVR = 0;
    //S32_SysTick->RVR = ((CLOCK_FREQ/1000) * quanta) - 1;
    //SYSPRI2 = (SYSPRI2 & 0x00FFFFFF) | 0xE0000000;
    //S32_SysTick->CSR = 0x7;

}


uint8_t osKernelAddThreads( void(*task0) (void),
                            void(*task1) (void),
                            void(*task2) (void),
                            void(*task3) (void))
{
    DISABLE_INTERRUPTS()

    /* Enchufar linked list  */
    tcbs[0].nextPtr = &tcbs[1];
    tcbs[1].nextPtr = &tcbs[2];
    tcbs[2].nextPtr = &tcbs[3];
    tcbs[3].nextPtr = &tcbs[0]; // Anus to mouth

    osKernelStackInit(0);
    TCB_Stack[0][STACK_SIZE-2] = (uint32_t)task0; /* Instalar PC del thread en el stack */

    osKernelStackInit(1);
    TCB_Stack[1][STACK_SIZE-2] = (uint32_t)task1; /* Instalar PC del thread en el stack */

    osKernelStackInit(2);
    TCB_Stack[2][STACK_SIZE-2] = (uint32_t)task2; /* Instalar PC del thread en el stack */

    osKernelStackInit(3);
    TCB_Stack[3][STACK_SIZE-2] = (uint32_t)task3; /* Instalar PC del thread en el stack */

    currentPtr = &tcbs[0]; /* Arrancar con el primero */
    ENABLE_INTERRUPTS()

    return 1;
}

__attribute__((naked)) void SysTick_Handler(void)
{
__asm volatile(

        "CPSID    I \n"
        "PUSH    {R4-R11} \n"
        "LDR     R0,=currentPtr \n"/* Cargar direcion del stack pointer actual */
        "LDR     R1,[R0]        \n"/* Cargar valor del currentPtr */
        "STR     SP,[R1]      \n" /* Actualizar currentPtr */

        /* Choose next ptr in list y cargar contexto */
        "LDR     R1,[R1,#4]   \n" /* Offset de 4 da el segundo elemento del tcb, que es el siguiente SP a usar */
        "STR     R1,[R0]      \n"  /* Actualizer currentPtr */
        "LDR     SP,[R1]      \n"  /* Actualizar SP */
        "POP     {R4-R11}     \n"  /* Restaurar contexto */
        "CPSIE   I              \n"
        "BX      LR         \n"    /* Salir y se restauran los demas registros */

);
}

__attribute__((naked)) void osSchedulerLaunch(void)
{
__asm volatile(

        //"CPSID   I  \n"
        "LDR     R0,=currentPtr \n" /* Cargar direccion de currentPtr */
        "LDR     R2,[R0] \n" /* Cargar valor de currentPtr */
        "LDR     SP,[R2] \n" /* Cargar valor de currentPtr en SP */

        /* Get initial stack frame, la inversa de lo que se hace por hardware */
        "POP {R4-R11} \n" /* Primero lo que se hace por software */
        "POP {R0-R3}  \n"/* hwr */
        "POP {R12}  \n" /* hwr */
        "ADD SP, SP, #4  \n"/* Brincar LR, porque ya esta iniciado */
        "POP {LR}  \n"/* Crear start location, cargando PC del stack al LR */
        "ADD SP, SP, #4  \n"/* Brincar xPSR, que es el ultimo elemento */
        "CPSIE  I \n"
        "BX  LR  \n"/* Brincar al PC que se cargó en LR */

);
}

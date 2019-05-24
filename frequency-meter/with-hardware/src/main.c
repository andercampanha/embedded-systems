#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c1294ncpdt.h" // CMSIS-Core
#include "inc/hw_memmap.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h" // driverlib
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"
#include "driverlib/uartstdio.h"

#define SECOND_SCALE 1
#define MILI_SECOND_SCALE 2

const int clockFrequency = 100000000;
int tickPeriod = 1000000;
int edge_count;
uint32_t frequency = 0;

void SysTick_Handler(void){
    if(edge_count > 0){
      edge_count = MAP_TimerValueGet(TIMER0_BASE, TIMER_A);
    }
} // SysTick_Handler

void rising_edge_timer_handler(){

}

void set_scale(int scale){  
  switch(scale){
  case 1:
    tickPeriod = 1;
    break;
  case 2:
    break;    
  default:
    break;
  } 
}

//Fun��o que configura o Timer 0A para funcionar como um contador de bordas de
//subida, utilizando o GPIO A Pino 0.
void init_timer_A0(void){
    // Habilita o GPIO Port A.
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); 

    // Habilita o Timer 0.
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); 

    // Configura a fun��o alternativa do GPIO Port A Pino 0
    MAP_GPIOPinConfigure(GPIO_PA0_T0CCP0);
    
    // Habilita a fun��o alternativa como Timer para o GPIO Port A Pino 0.
    MAP_GPIOPinTypeTimer(GPIO_PORTA_BASE, GPIO_PIN_0);

    //Configura o Timer 0 como sendo um Timer A e um B. O Timer A � configurado
    //para contar bordas de forma crescente e definido para 16/24 bits (prescale).
    MAP_TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_COUNT_UP));
    
    //Registra uma fun��o para lidar com a interrup��o do timer. 
    TimerIntRegister(TIMER0_BASE, TIMER_A, rising_edge_timer_handler);
    
    //Define o TIPO de evento que o Timer deve gerenciar. Por tipo de evento se
    //entende se � uma Borda de Subida, Descida ou ambas as bordas.
    MAP_TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);    
    
    //Define a origem e tipo das interrup��es para o Timer 0A. Por origem se 
    //entende qual Timer (A ou B) ser� monitorado. Por tipo se entende se a
    //interrup��o � para Timeout, evento, RTC, etc.
    MAP_TimerIntEnable(TIMER0_BASE, TIMER_CAPA_EVENT);
    
    //Limpa o buffer de escrita. Necess�rio para garantir as altera��es aplicadas
    //na fun��o "ROM_TimerIntEnable".
    MAP_TimerIntClear(TIMER0_BASE, TIMER_CAPA_EVENT);
    
    //Habilita interrup��es para o Timer 0A.
    MAP_IntEnable(INT_TIMER0A); 
    
    //Habilita as interrup��es para o processador, ou seja,
    //permite ao processador responder a interrup��es
    MAP_IntMasterEnable(); 
    
    //Habilita o funcionamento do Timer.
    //Ap�s essa chamada o timer iniciar� sua contagem efetivamente.
    MAP_TimerEnable(TIMER0_BASE, TIMER_A);     
}

//Inicializa a interface UART no Port B Pino 0 (Receive) e Pino 1 (Transmit).
void init_UART_1(void) { 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
    UARTprintf("UART started\n");
}

//Inicializa o Push Button SW1 no GPIO J Pino 0.
void init_push_button(){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Habilita GPIO J (push-button SW1 = PJ0)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilita��o
    
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0); // push-buttons SW1
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

int main()
{
  uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                            SYSCTL_OSC_MAIN |
                                            SYSCTL_USE_PLL |
                                            SYSCTL_CFG_VCO_480),
                                            clockFrequency); // PLL em 100MHz
  
  SysTickEnable();
  SysTickPeriodSet(tickPeriod); // TODO: Verificar l�gica para definir Periodo do Tick
  SysTickIntEnable();
  
  init_timer_A0();
  init_UART_1();
  init_push_button();
  
  while(1){
    if(GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) != GPIO_PIN_0){ // Testa estado do push-button SW1
      //TODO trocar de escala.
    }
    
    frequency = (edge_count)/(tickPeriod);
    
    UARTprintf("frequency: %d; edge_count: %d; clock_count: %d\n", frequency, edge_count, clock_count);
  }
  
}

// http://e2e.ti.com/support/microcontrollers/other/f/908/t/735931?TM4C129ENCPDT-TIVA-TM4C1294-configuring-single-24-bit-capture-counter-
// http://dev.ti.com/tirex/content/simplelink_msp432e4_sdk_2_40_00_11/docs/driverlib/msp432e4/html/group__interrupt__api.html#ga49fc9c3d1a0f8c42a20249f8c5d360ce
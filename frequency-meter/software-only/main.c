#include <stdbool.h>
#include <stdint.h>
#include "inc/tm4c1294ncpdt.h" // CMSIS-Core
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h" // driverlib
#include "driverlib/gpio.h"

#include "driverlib/uart.h"
#include "driverlib/uartstdio.h"

#include "driverlib/pin_map.h"

void initUART(void) { 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
    UARTprintf("UART started\n");
}

void main(void){
  int keep_pooling;
  uint32_t pulse_count;
  uint32_t sample_number;
  int wave_state = 1;
  uint32_t frequency = 0;
  uint32_t metric = 0;
  uint32_t time_base = 0;
  
  const int clock_frequency = 24000000;
  
  uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                              SYSCTL_OSC_MAIN |
                                              SYSCTL_USE_PLL |
                                              SYSCTL_CFG_VCO_480),
                                              clock_frequency); // PLL em 24MHz
  initUART();
  
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
  
  GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_7);
  GPIOPadConfigSet(GPIO_PORTK_BASE, GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Habilita GPIO J (push-button SW1 = PJ0, push-button SW2 = PJ1)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilitação
    
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0); // push-buttons SW1 e SW2 como entrada
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

  while(1) {

    keep_pooling = 1;
    pulse_count = 0;
    sample_number = 0;

    while(GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_7) == GPIO_PIN_7);
    
    while(keep_pooling == 1) {
      sample_number++;

      int aux = GPIOPinRead(GPIO_PORTK_BASE, GPIO_PIN_7);
      if (wave_state == 0 && aux == GPIO_PIN_7) {
        pulse_count++;
      } else {
        metric++;
      }

      wave_state = aux;

      if (sample_number < 1000000) {
        keep_pooling = 1;
      } else {
        keep_pooling = wave_state;
      }
    }

    if(GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) != GPIO_PIN_0) // Testa estado do push-button SW1
      time_base = !time_base;

    frequency = (pulse_count/31)*(clock_frequency/sample_number);

    if (time_base)
    	frequency = frequency/1000;

    UARTprintf("frequency: %d; sample_number: %d; pulse_count: %d\n", frequency, sample_number, pulse_count);
  }
} // main

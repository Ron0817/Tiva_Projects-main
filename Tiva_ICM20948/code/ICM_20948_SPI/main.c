#include "nrf24l01.h"


uint32_t ui32TxBuffer[MAX_PLOAD];
uint32_t ui32RxBuffer[MAX_PLOAD];

// Define pin to LED color mapping.
#define LED_0   GPIO_PIN_0
#define LED_1   GPIO_PIN_1
#define LED_2   GPIO_PIN_2
#define LED_3   GPIO_PIN_3

// The error routine that is called if the driver library encounters an error.
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

void ConfigureUART(void)
{
    // Enable the GPIO Peripheral used by the UART.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable UART0
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    ROM_SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}


void count_to_message(uint32_t count_val, uint32_t ui32TxBuffer[MAX_PLOAD]){
    //uint32_t * messagePtr = (uint32_t*)malloc(sizeof(uint32_t)*MAX_PLOAD);
    //divide the count value into bytes
    uint8_t byte0 = count_val & 0xFF;
    count_val = count_val >> 8;
    uint8_t byte1 = count_val & 0xFF;
    count_val = count_val >> 8;
    uint8_t byte2 = count_val & 0xFF;
    count_val = count_val >> 8;
    uint8_t byte3 = count_val & 0xFF;
    count_val = count_val >> 8;
    uint8_t byte4 = count_val & 0xFF;
    count_val = count_val >> 8;
    uint8_t byte5 = count_val & 0xFF;
    count_val = count_val >> 8;
    ui32TxBuffer[0] = byte0;
    ui32TxBuffer[1] = byte1;
    ui32TxBuffer[2] = byte2;
    ui32TxBuffer[3] = byte3;
    ui32TxBuffer[4] = byte4;
    ui32TxBuffer[5] = byte5;
}

int main(void)
{
    uint32_t i;

    // Setup the system clock to run at 50 Mhz from PLL with external oscillator
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);


    // Launchpad Board
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
    ROM_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure PB2 as an output pin.
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);

    // configure UART for console operation
    ConfigureUART();
    /*SSI3 Signals:
    PD0 -> SSI3CLK
    PD2 -> SSI3RX (MISO)
    PD3 -> SSI3TX (MOSI)
     */
    SPIInit();
    uint32_t returnVal;
    while(1)
    {
        //SPIDataWrite(0x5);
        //read register 0 -> 10000000
        returnVal = ICM_SPI_Read(0x0);
        UARTprintf("0x%x\n",returnVal);
        ROM_SysCtlDelay(SysCtlClockGet()); //lower diviser doesn't work properly
    }
}


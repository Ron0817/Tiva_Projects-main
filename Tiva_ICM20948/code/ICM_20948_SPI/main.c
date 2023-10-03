#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "utils/uartstdio.h"
#include "spi.h"
#include "icm20948.h"

/* -----------------------          Define Macros       --------------------- */
// Defind Max ui32TxBuffer size
#define MAX_PLOAD 32

// Define pin to LED color mapping.
#define LED_0   GPIO_PIN_0
#define LED_1   GPIO_PIN_1
#define LED_2   GPIO_PIN_2
#define LED_3   GPIO_PIN_3

/* -----------------------      Global Variables        --------------------- */
uint32_t ui32TxBuffer[MAX_PLOAD];
uint32_t ui32RxBuffer[MAX_PLOAD];

// Variable to store the pin status of GPIO Port
uint32_t GPIOPinStatus = 0x00000000;

// While (!stop) {}
bool stop = 0;

/* -----------------------          Functions        --------------------- */
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

void GPIOPortFHandler(void)
{
    UARTprintf("Entered GPIOF IRQ \n ");
    stop = 1;

    // Clear interrupt flag
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
    UARTprintf("Exit GPIOF IRQ \n ");
}

/* -----------------------          Main Program        --------------------- */
// Add if #debug tag to all uartprintf()
int main(void)
{
    uint32_t i;
    uint32_t ret;
    uint32_t accel_x_hi;

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
    UARTprintf("UART was configured. Let's start\n");

    // TODO: Put them in SPI_init()
    /* -----------------------          SPI init        --------------------- */
    // SSI3 Signals:
    // PD0 -> SSI3CLK
    // PD2 -> SSI3RX (MISO)
    // PD3 -> SSI3TX (MOSI)
    SPIInit();
    ROM_SysCtlDelay(SysCtlClockGet()); //lower diviser doesn't work properly

    /* -----------------------          Pushbutton Interrupt Init        --------------------- */
    // TODO: Put in SW_int_init()
    // Remove the Lock present on Switch SW2 (connected to PF0) and commit the change
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= GPIO_PIN_0 | GPIO_PIN_4;

    // Set the System clock to 80MHz and enable the clock for peripheral PortF.
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Configure input for PF4(SW1) and PF0(SW2)
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);

    // Set up IRQ for PortF 4 |0
    GPIOIntRegister(GPIO_PORTF_BASE, GPIOPortFHandler);
    ROM_GPIOIntTypeSet(GPIO_PORTF_BASE,  GPIO_PIN_4 | GPIO_PIN_0, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);

    // Connect PF0, PF4 to internal Pull-up resistors and set 2 mA as current strength.
    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    /* -----------------------          Accel and Gyro init        --------------------- */
//    icm20948_init();

    /* -----------------------          ICM communication check        --------------------- */
    // Read Who am I reg
//    ret = icm20948_who_am_i();
    ret = ICM_SPI_Read(0);
    UARTprintf("ICM20948 is 0xea ? -> 0x%x\n", ret);
//    to delete
//    ICM_SPI_Write(0x7F, 2<<4);
//
//    ret = ICM_SPI_Read(0);
//    UARTprintf("ub2 reg0 0xea = 0x%x\n", ret);
//
//    ICM_SPI_Write(0x7F, 0<<4);
//
//    ret = ICM_SPI_Read(0);
//    UARTprintf("back to ub0 reg0 = 0x%x\n", ret);
//
    ret = ICM_SPI_Read(0x06);
    UARTprintf("wakeup x= 0x%x\n", ret);

    ICM_SPI_Write(0x06, 0x01);
    ret = ICM_SPI_Read(0x06);
    UARTprintf("wakeup 01= 0x%x\n", ret);

    ICM_SPI_Write(0x06, 0x40);
    ret = ICM_SPI_Read(0x06);
    UARTprintf("wakeup 40= 0x%x\n", ret);

    ICM_SPI_Write(0x06, 0x01);
    ret = ICM_SPI_Read(0x06);
    UARTprintf("wakeup 01= 0x%x\n", ret);
//
//    ret = ICM_SPI_Read(0x06);
//    UARTprintf("wakeup 41= 0x%x\n", ret);
//

    while (!stop)
    {
        // TODO: Put them in read_accel()
        // Read Accel X-axis
        ret = ICM_SPI_Read(0x2D) << 8;
        UARTprintf("accel_x_hi = 0x%x\n",ret);
        ret = ret | ICM_SPI_Read(0x2E);
        UARTprintf("accel_x_lo = 0x%x\n",ret);
        ROM_SysCtlDelay(SysCtlClockGet() / 5);
    }

    UARTprintf("Done\n");

    return 0;
}


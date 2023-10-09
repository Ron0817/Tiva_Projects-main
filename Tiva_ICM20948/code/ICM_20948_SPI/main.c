
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "driverlib/timer.h"
#include "spi.h"
#include "icm20948.h"

/* ------------------------------------          Define Macros       ---------------------------------- */
// Defind Max ui32TxBuffer size
#define MAX_PLOAD 32

// Define pin to LED color mapping.
#define LED_0   GPIO_PIN_0
#define LED_1   GPIO_PIN_1
#define LED_2   GPIO_PIN_2
#define LED_3   GPIO_PIN_3

/* ------------------------------------      Global Variables        ---------------------------------- */
uint32_t ui32TxBuffer[MAX_PLOAD];
uint32_t ui32RxBuffer[MAX_PLOAD];

// Variable to store the pin status of GPIO Port
uint32_t GPIOPinStatus = 0x00000000;

// While (!stop) {}
bool stop = 0;

// gyro and accel axises
axises gyro_axises;
axises accel_axises;
// For UARTprint =
axises_str gyro_axises_str;
axises_str accel_axises_str;
// Workaround for sprintf() to UARTprint 2's complement number
struct axises_sign{
    char x[2];
    char y[2];
    char z[2];
};


/* ------------------------------------          Functions        ---------------------------------- */
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

/* ------------------------------------          Timers        ---------------------------------- */
// Timer5 is used for triggering ICM gyro and accel SPI reading
void timer5_init(uint32_t frequency)
{
    UARTprintf("**************Enter timer init\n");
    // Enable the timer peripheral
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);

    // Timer should run periodically
    // Full-width periodic timer
    ROM_TimerConfigure(TIMER5_BASE, TIMER_CFG_PERIODIC);

    // Set the compare value of the timer
    // This corresponds to the sampling frequency
    ROM_TimerLoadSet(TIMER5_BASE, TIMER_A, (ROM_SysCtlClockGet()/frequency)-1);

    // Enable the timer interrupt
    ROM_TimerIntEnable(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    ROM_IntEnable(INT_TIMER5A);

    // Enable Timer5
    ROM_TimerEnable(TIMER5_BASE, TIMER_A);

}

/* ------------------------------------          Interrupts        ---------------------------------- */
// SW 1 and 2 are used to stop the main while loop
void GPIOPortFHandler(void)
{
    UARTprintf("Entered GPIOF IRQ \n ");
    stop = 1;

    // Clear interrupt flag
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0);
    UARTprintf("Exit GPIOF IRQ \n ");
}

// Handler for timer 5 to read ICM gyro and accel readings
void Timer5IntHandler(void)
{
    static char* ret_val;
    // Workaround for sprintf() to UARTprint 2's complement number
    static struct axises_sign gyro_sign;
    static struct axises_sign accel_sign;

    // SW interrupt to stop the program
    if (stop)
    {
        ROM_TimerDisable(TIMER5_BASE, TIMER_A);
    }

    // clear the timer interrupt
    ROM_TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);

    // Read gyro and accel axises
    icm20948_gyro_read_dps(&gyro_axises);
    icm20948_accel_read_g(&accel_axises);

    // Interesting findings that the following sprintf() doesn't work in this ISR.
    // Reasons might be that it hangs the ISR and takes too long time.
    // Workaround was used to hardcode the sign of gyro and accel readings
//    ret_val = (uint16_t) gyro_axises.x < 0x1fff ? sprintf(gyro_axises_str.x, "%d", (uint16_t) gyro_axises.x)
//            : sprintf(gyro_axises_str.x, "-%d", 0xffff - (uint16_t) gyro_axises.x);
//    ret_val = (uint16_t) gyro_axises.y < 0x1fff ? sprintf(gyro_axises_str.y, "%d", (uint16_t) gyro_axises.y)
//            : sprintf(gyro_axises_str.y, "-%d", 0xffff - (uint16_t) gyro_axises.y);
//    ret_val = (uint16_t) gyro_axises.z < 0x1fff ? sprintf(gyro_axises_str.z, "%d", (uint16_t) gyro_axises.z)
//            : sprintf(gyro_axises_str.z, "-%d", 0xffff - (uint16_t) gyro_axises.z);
//
//    ret_val = (uint16_t) accel_axises.x < 0x1fff ? sprintf(accel_axises_str.x, "%d", (uint16_t) accel_axises.x)
//            : sprintf(accel_axises_str.x, "-%d", 0xffff - (uint16_t) accel_axises.x);
//    ret_val = (uint16_t) accel_axises.y < 0x1fff ? sprintf(accel_axises_str.y, "%d", (uint16_t) accel_axises.y)
//            : sprintf(accel_axises_str.y, "-%d", 0xffff - (uint16_t) accel_axises.y);
//    ret_val = (uint16_t) accel_axises.z < 0x1fff ? sprintf(accel_axises_str.z, "%d", (uint16_t) accel_axises.z)
//            : sprintf(accel_axises_str.z, "-%d", 0xffff - (uint16_t) accel_axises.z);

//    UARTprintf("gyro_val (x, y, z) = (%s,\t%s,\t%s) \t", gyro_axises_str.x, gyro_axises_str.y, gyro_axises_str.z);
//    UARTprintf("accel_val (x, y, z) = (%s,\t%s,\t%s) \n", accel_axises_str.x, accel_axises_str.y, accel_axises_str.z);

    // Convert 2's comp num and print
    (uint16_t) gyro_axises.x < 0x1fff ? strcpy(gyro_sign.x, "+") : strcpy(gyro_sign.x, "-");
    (uint16_t) gyro_axises.y < 0x1fff ? strcpy(gyro_sign.y, "+") : strcpy(gyro_sign.y, "-");
    (uint16_t) gyro_axises.z < 0x1fff ? strcpy(gyro_sign.z, "+") : strcpy(gyro_sign.z, "-");
    gyro_axises.x = (uint16_t) gyro_axises.x < 0x1fff ? (uint16_t) gyro_axises.x : 0xffff - (uint16_t) gyro_axises.x;
    gyro_axises.y = (uint16_t) gyro_axises.y < 0x1fff ? (uint16_t) gyro_axises.y : 0xffff - (uint16_t) gyro_axises.y;
    gyro_axises.z = (uint16_t) gyro_axises.z < 0x1fff ? (uint16_t) gyro_axises.z : 0xffff - (uint16_t) gyro_axises.z;

    (uint16_t) accel_axises.x < 0x1fff ? strcpy(accel_sign.x, "+") : strcpy(accel_sign.x, "-");
    (uint16_t) accel_axises.y < 0x1fff ? strcpy(accel_sign.y, "+") : strcpy(accel_sign.y, "-");
    (uint16_t) accel_axises.z < 0x1fff ? strcpy(accel_sign.z, "+") : strcpy(accel_sign.z, "-");
    accel_axises.x = (uint16_t) accel_axises.x < 0x1fff ? (uint16_t) accel_axises.x : 0xffff - (uint16_t) accel_axises.x;
    accel_axises.y = (uint16_t) accel_axises.y < 0x1fff ? (uint16_t) accel_axises.y : 0xffff - (uint16_t) accel_axises.y;
    accel_axises.z = (uint16_t) accel_axises.z < 0x1fff ? (uint16_t) accel_axises.z : 0xffff - (uint16_t) accel_axises.z;

    // Print
    UARTprintf("gyro_val (x, y, z) = (%s%d,\t%s%d,\t%s%d) \t", gyro_sign.x, (uint16_t) gyro_axises.x,
               gyro_sign.y, (uint16_t) gyro_axises.y, gyro_sign.y, (uint16_t) gyro_axises.z);
    UARTprintf("accel_val (x, y, z) = (%s%d,\t%s%d,\t%s%d) \n", accel_sign.x, (uint16_t) accel_axises.x,
                   accel_sign.y, (uint16_t) accel_axises.y, accel_sign.y, (uint16_t) gyro_axises.z);
}




/* ------------------------------------          Main Program        ---------------------------------- */
// Add if #debug tag to all uartprintf()
int main(void)
{
    uint32_t i;
    uint32_t ret;
    uint32_t ICM_sampling_frequency = 100;

//    // gyro and accel axises
//    axises gyro_axises;
//    axises accel_axises;
//
//    // For UARTprint only
//    axises_str gyro_axises_str;
//    axises_str accel_axises_str;

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

    /* ------------------------------------          SPI init        ---------------------------------- */
    // SSI3 Signals:
    // PD0 -> SSI3CLK
    // PD2 -> SSI3RX (MISO)
    // PD3 -> SSI3TX (MOSI)
    SPIInit();
    ROM_SysCtlDelay(SysCtlClockGet()/10); //lower diviser doesn't work properly


    /* ------------------------------------          Pushbutton Interrupt Init        ---------------------------------- */
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

    /* ------------------------------------          Accel and Gyro init        ---------------------------------- */
    ret = icm20948_who_am_i();
    UARTprintf("ICM20948 is 0xea ? -> 0x%x\n", ret);
    icm20948_init();

    UARTprintf("Init check done. Start Reading ...\n");
    ROM_SysCtlDelay(SysCtlClockGet());

    /* ------------------------------------          Timer5 init        ---------------------------------- */
    timer5_init(ICM_sampling_frequency);

    while (!stop)
    {

//        UARTprintf("Still alive...\n");
        ROM_SysCtlDelay(SysCtlClockGet() / 20);
//        icm20948_gyro_read_dps(&gyro_axises);
//        icm20948_accel_read_g(&accel_axises);

//        UARTprintf("gyro_val (x, y, z) = (%d,\t%d,\t%d) \n", (uint16_t)gyro_axises.x, (uint16_t)gyro_axises.y, (uint16_t)gyro_axises.z);
        // Add offset for 2's comp negative values
//        ret = (uint16_t) gyro_axises.x < 0x1fff ? sprintf(gyro_axises_str.x, "%d", (uint16_t) gyro_axises.x)
//                : sprintf(gyro_axises_str.x, "-%d", 0xffff - (uint16_t) gyro_axises.x);
//        ret = (uint16_t) gyro_axises.y < 0x1fff ? sprintf(gyro_axises_str.y, "%d", (uint16_t) gyro_axises.y)
//                : sprintf(gyro_axises_str.y, "-%d", 0xffff - (uint16_t) gyro_axises.y);
//        ret = (uint16_t) gyro_axises.z < 0x1fff ? sprintf(gyro_axises_str.z, "%d", (uint16_t) gyro_axises.z)
//                : sprintf(gyro_axises_str.z, "-%d", 0xffff - (uint16_t) gyro_axises.z);
//
//        ret = (uint16_t) accel_axises.x < 0x1fff ? sprintf(accel_axises_str.x, "%d", (uint16_t) accel_axises.x)
//                : sprintf(accel_axises_str.x, "-%d", 0xffff - (uint16_t) accel_axises.x);
//        ret = (uint16_t) accel_axises.y < 0x1fff ? sprintf(accel_axises_str.y, "%d", (uint16_t) accel_axises.y)
//                : sprintf(accel_axises_str.y, "-%d", 0xffff - (uint16_t) accel_axises.y);
//        ret = (uint16_t) accel_axises.z < 0x1fff ? sprintf(accel_axises_str.z, "%d", (uint16_t) accel_axises.z)
//                : sprintf(accel_axises_str.z, "-%d", 0xffff - (uint16_t) accel_axises.z);
//
//        UARTprintf("gyro_val (x, y, z) = (%s,\t%s,\t%s) \t", gyro_axises_str.x, gyro_axises_str.y, gyro_axises_str.z);
//        UARTprintf("accel_val (x, y, z) = (%s,\t%s,\t%s) \n", accel_axises_str.x, accel_axises_str.y, accel_axises_str.z);

    }

    UARTprintf("Done\n");

    return 0;
}


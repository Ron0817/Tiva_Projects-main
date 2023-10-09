#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
    axises gyro_axises;
    axises accel_axises;

    // To be deleted
    uint16_t gyro_x;
    uint16_t gyro_y;
    uint16_t gyro_z;
    uint16_t accel_x;
    uint16_t accel_y;
    uint16_t accel_z;

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

    /* -----------------------          SPI init        --------------------- */
    // SSI3 Signals:
    // PD0 -> SSI3CLK
    // PD2 -> SSI3RX (MISO)
    // PD3 -> SSI3TX (MOSI)
    SPIInit();
    ROM_SysCtlDelay(SysCtlClockGet()/10); //lower diviser doesn't work properly

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
    ret = icm20948_who_am_i();
    UARTprintf("ICM20948 is 0xea ? -> 0x%x\n", ret);
    icm20948_init();

    UARTprintf("Init check done. Start Reading ...\n");
    ROM_SysCtlDelay(SysCtlClockGet());

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
//    ret = ICM_SPI_Read(0x06);
//    UARTprintf("wakeup x= 0x%x\n", ret);
//
//    ICM_SPI_Write(0x06, 0x01);
//    ret = ICM_SPI_Read(0x06);
//    UARTprintf("wakeup 01= 0x%x\n", ret);
//
//    ICM_SPI_Write(0x06, 0x40);
//    ret = ICM_SPI_Read(0x06);
//    UARTprintf("wakeup 40= 0x%x\n", ret);
//
//    ICM_SPI_Write(0x06, 0x01);
//    ret = ICM_SPI_Read(0x06);
//    UARTprintf("wakeup 01= 0x%x\n", ret);
//
//    ret = ICM_SPI_Read(0x06);
//    UARTprintf("wakeup 41= 0x%x\n", ret);

    // For UARTprint only
    axises_str gyro_axises_str;
    axises_str accel_axises_str;

    while (!stop)
    {
        // TODO: Put them in read_accel()
//        ICM_SPI_Write(0x7F, 0 << 4);

        // Read Accel val
//        accel_x = ICM_SPI_Read(0x2D) << 8;
//        accel_x = accel_x | ICM_SPI_Read(0x2E);
//
//        accel_y = ICM_SPI_Read(0x2F) << 8;
//        accel_y = ret | ICM_SPI_Read(0x30);
//
//        accel_z = ICM_SPI_Read(0x31) << 8;
//        accel_z = ret | ICM_SPI_Read(0x32);

//        // Read Gyro val
//        gyro_x = ICM_SPI_Read(0x33) << 8;
//        gyro_x = gyro_x | ICM_SPI_Read(0x34);
//
//        gyro_y = ICM_SPI_Read(0x35) << 8;
//        gyro_y = gyro_y | ICM_SPI_Read(0x36);
//
//        gyro_z = ICM_SPI_Read(0x37) << 8;
//        gyro_z = gyro_z | ICM_SPI_Read(0x38);
//
//        UARTprintf("gyro_val (x, y, z) = (%x, %x, %x) \t",gyro_x ,gyro_y ,gyro_z);
//        UARTprintf("accel_val (x, y, z) = (%d, %d, %d) \n",accel_x/512 ,accel_y/512 ,accel_z/512);


        ROM_SysCtlDelay(SysCtlClockGet() / 20);
        icm20948_gyro_read_dps(&gyro_axises);
        icm20948_accel_read_g(&accel_axises);


        // Add offset for 2's comp negative values
        ret = (uint16_t) gyro_axises.x < 0x1fff ? sprintf(gyro_axises_str.x, "%d", (uint16_t) gyro_axises.x)
                : sprintf(gyro_axises_str.x, "-%d", 0xffff - (uint16_t) gyro_axises.x);
        ret = (uint16_t) gyro_axises.y < 0x1fff ? sprintf(gyro_axises_str.y, "%d", (uint16_t) gyro_axises.y)
                : sprintf(gyro_axises_str.y, "-%d", 0xffff - (uint16_t) gyro_axises.y);
        ret = (uint16_t) gyro_axises.z < 0x1fff ? sprintf(gyro_axises_str.z, "%d", (uint16_t) gyro_axises.z)
                : sprintf(gyro_axises_str.z, "-%d", 0xffff - (uint16_t) gyro_axises.z);

        ret = (uint16_t) accel_axises.x < 0x1fff ? sprintf(accel_axises_str.x, "%d", (uint16_t) accel_axises.x)
                : sprintf(accel_axises_str.x, "-%d", 0xffff - (uint16_t) accel_axises.x);
        ret = (uint16_t) accel_axises.y < 0x1fff ? sprintf(accel_axises_str.y, "%d", (uint16_t) accel_axises.y)
                : sprintf(accel_axises_str.y, "-%d", 0xffff - (uint16_t) accel_axises.y);
        ret = (uint16_t) accel_axises.z < 0x1fff ? sprintf(accel_axises_str.z, "%d", (uint16_t) accel_axises.z)
                : sprintf(accel_axises_str.z, "-%d", 0xffff - (uint16_t) accel_axises.z);

        UARTprintf("gyro_val (x, y, z) = (%s,\t%s,\t%s) \t", gyro_axises_str.x, gyro_axises_str.y, gyro_axises_str.z);
        UARTprintf("accel_val (x, y, z) = (%s,\t%s,\t%s) \n", accel_axises_str.x, accel_axises_str.y, accel_axises_str.z);

    }

    UARTprintf("Done\n");

    return 0;
}


#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"
#include "utils/uartstdio.h"
#include "usb_structs.h"
#include "usbconfig.h"
#include "nrf24l01.h"
#include "spi.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "driverlib/fpu.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"


//#include "utils/cmdline.h"
//#include "utils/uartstdio.h"
//#include "utils/uartstdio.c"
#include "driverlib/adc.h"
//#include "inc/hw_ssi.h"
#include "driverlib/debug.h"
//#include "ff.h"
#include "diskio.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/cmdline.h"
//#include "utils/uartstdio.h"
//#include "utils/uartstdio.c"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ints.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/ssi.h"
#include "ff.h"
#include "diskio.h"

// Defines for task
//#define LED_0 GPIO_PIN_4 // on board LED
#define BUFFER_A 0
#define BUFFER_B 1

// Define pin to LED color mapping.
#define LED_0   GPIO_PIN_0
#define LED_1   GPIO_PIN_1
#define LED_2   GPIO_PIN_2
#define LED_3   GPIO_PIN_3

// buffers for raw data
uint16_t *bufferA;
uint16_t *bufferB;
uint32_t buffer_size;

// variable to keep track of active buffer
volatile bool bufferA_empty;
volatile bool bufferB_empty;
volatile bool buffer_mode;
volatile bool storage_on;

int n;
uint32_t pui32DataRx[1];
uint32_t channel_number;


#define DEBUG 1;

// the error routine that is called if the driver library encounters an error
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

/*Variable required for SD Card R/W*/
FATFS fatfs;
FIL fil;
FRESULT rc;
UINT br, bw;

//defines for sampling paramters since there is no sd card reading for params anymore
#define    SAMPLING_FREQ     5000
#define    BUFFER_SIZE       16           //16*2bytes per sample = 32 byte payload
#define    CUTOFF_UPPER      20000
#define    CUTOFF_LOWER      1
#define    CHANNEL_NUM       8
#define    FILE_SECONDS      5
//#define    FILENAME          0123_


void RxDataHandler();

// systick handler for FatFs library
void
SysTickHandler(void)
{
    ROM_SysTickIntDisable();
    disk_timerproc();
    ROM_SysTickIntEnable();
}

// function to send data to headstage
void SPI_Send(uint32_t data_MS, uint32_t data_LS)
{
    // pull-down the FSS signal and delay for some time
    ROM_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0);
    ROM_SysCtlDelay(1);

    ROM_SSIDataPut(SSI0_BASE, data_MS);
    ROM_SSIDataPut(SSI0_BASE, data_LS);

    // wait until the data is fully transferred
    while(ROM_SSIBusy(SSI0_BASE))
    {
    }

    // delay for some time and pull-up the FSS signal
    ROM_SysCtlDelay(1);
    ROM_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0b1000000);
}

// function to initialize onboard UART.
void uart_init(void)
{
    // enable uart gpio.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // configure gpio pins as uart.
    // Port A0 is used for UART data receiving, and A1 is used for UART data transmitting
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);

    // enable the uart module.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // set clock for baud rate generation.
    ROM_UARTClockSourceSet(UART0_BASE, UART_CLOCK_SYSTEM);

    // set pin type to uart.
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // configure uart 0 at 115200 bps.
    UARTStdioConfig(0, 115200, SysCtlClockGet());
}


// Timer0 is used for triggering SPI data transfer
void timer0_init(uint32_t frequency)
{
    // Enable the timer peripheral
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Timer should run periodically
    // Full-width periodic timer
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // Set the compare value of the timer
    // This corresponds to the sampling frequency
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, (ROM_SysCtlClockGet()/frequency)-1);

    // Enable the timer interrupt
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_IntEnable(INT_TIMER0A);

    // Enable Timer0
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
}

// function to initialize the headstage using SPI
int headstage_init(void)
{
    uint32_t DataRx_init[1]; // variable for receiving data

    // "Dummy" command, read the ROM value of 255;
    SPI_Send(0b1100000011111111, 0b0000000000000000); // 1100 0000 1111 1111,

    // clear the receive FIFO
    while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]))
    {
    }

    // disable stimulation
    // write(32, 0x0000)
    SPI_Send(0b1000000000100000, 0b0000000000000000); //1000 0000 0010 0000,

    // clear the receive FIFO
    while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]))
    {
    }

    // write(33, 0x0000)
    SPI_Send(0b1000000000100001, 0b0000000000000000); //1000 0000 0010 0001,

    // get the return value of the first command
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    // check the received data
    if(! (DataRx_init[1] == 0) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0b0000000000100000) ){ //0000 0000 0010 0000
        return 0;
    }else{}

    // Power up all DC-coupled low-gain amplifiers
    // Write(38, 0xFFFF)
    SPI_Send(0b1000000000100110, 0xFFFF); //1000 0000 0010 0110

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0) ){
        return 0;
    }else{}


    // run the CLEAR command
    SPI_Send(0b0110101000000000, 0b0000000000000000); //0110 1010 0000 0000

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0) ){
        return 0;
    }else{}

    // write register 0, set the bias current for ADC and MUX
    // total ADC sampling rate is 480KS/s
    SPI_Send(0b1000000000000000, 0x00C7); //1000 0000 0000 0000

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0xFFFF) ){
        return 0;
    }else{}


    // write register 1, set all auxiliary digital outputs to high-impedance;
    // set DSP high-pass filter to 4.665Hz
    SPI_Send(0b1000000000000001, 0x051A); //1000 0000 0000 0001

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! ((DataRx_init[1] & 0x7FFF) == 0) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0) ){
        return 0;
    }else{}


    // write register 2
    // power up DAC used for impedance testing, but disable impedance testing
    SPI_Send(0b1000000000000010, 0x0040); //1000 0000 0000 0010

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x00C7) ){
        return 0;
    }else{}

    // write register 3
    // initialize impedance check DAC to midrange value
    SPI_Send(0b1000000000000011, 0x0080); //1000 0000 0000 0011

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x051A) ){
        return 0;
    }else{}

    // write register 4 and 5
    // set upper cutoff frequency of AC-coupled high-gain amplifiers to 7.5kHz
    SPI_Send(0b1000000000000100, 0x0016); //1000 0000 0000 0100

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0040) ){
        return 0;
    }else{}

    SPI_Send(0b1000000000000101, 0x0017); //1000 0000 0000 0101

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0080) ){
        return 0;
    }else{}

    // write register 6, set lower cutoff frequency to 5Hz
    SPI_Send(0b1000000000000110, 0x00A8); //1000 0000 0000 0110

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0016) ){
        return 0;
    }else{}

    // write register 7, set alternate lower cutoff frequency to 1000Hz
    SPI_Send(0b1000000000000111, 0x000A); //1000 0000 0000 0111

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0017) ){
        return 0;
    }else{}

    // write register 8, power up all AC-coupled high-gain amplifiers
    SPI_Send(0b1000000000001000, 0xFFFF); //1000 0000 0000 1000

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x00A8) ){
        return 0;
    }else{}

    // write register 10, turn off fast settle function on all channels
    // U flag controlled; U = 1;
    SPI_Send(0b1010000000001010, 0x0000); //1010 0000 0000 1010

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x000A) ){
        return 0;
    }else{}

    // write register 12, set all amplifiers to the lower cutoff
    // frequency set by register 6; U flag controlled, U = 1
    SPI_Send(0b1010000000001100, 0xFFFF); //1010 0000 0000 1100

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0xFFFF) ){
        return 0;
    }else{}

    // write register 34, set up a stimulation step size of 1uA;
    // this will give a stimulation range of +-255uA on each channel; U = 1
    SPI_Send(0b1010000000100010, 0x00E2); //1010 0000 0010 0010

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0000) ){
        return 0;
    }else{}

    // write register 35, set stimulation bias voltages appropriate for a 1uA step size
    SPI_Send(0b1000000000100011, 0x00AA); //1000 0000 0010 0011

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0xFFFF) ){
        return 0;
    }else{}

    // write register 36, set current-limited charge recovery target voltage to zero
    SPI_Send(0b1000000000100100, 0x0080); //1000 0000 0010 0100

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x00E2) ){
        return 0;
    }else{}

    // write register 37, set charge recovery current limit to 1 nA
    SPI_Send(0b1000000000100101, 0x4F00); //1000 0000 0010 0101

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x00AA) ){
        return 0;
    }else{}

    // write register 42, turn off all stimulators
    // U flag controlled, U = 1
    SPI_Send(0b1010000000101010, 0x0000); //1010 0000 0010 1010

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0080) ){
        return 0;
    }else{}

    // write register 44, set all stimulators to negative polarity
    // U flag controlled, U = 1
    SPI_Send(0b1010000000101100, 0x0000); //1010 0000 0010 1100

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x4F00) ){
        return 0;
    }else{}

    // write register 46, open all charge recovery switches
    // U flag controlled, U = 1
    SPI_Send(0b1010000000101110, 0x0000); //1010 0000 0010 1110

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0000) ){
        return 0;
    }else{}

    // write register 48, disable all current-limited charge recovery circuits
    // U flag controlled, U = 1
    SPI_Send(0b1010000000110000, 0x0000); //1010 0000 0011 0000

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0000) ){
        return 0;
    }else{}

    // write registers 64-79, setting the negative stimulation current magnitudes
    // to zero and the current trims to the center point
    // U flag controlled, U = 1
    uint32_t reg_number;

    for(reg_number = 0b01000000 ; reg_number <= 0b01001111; reg_number ++)
    {
        SPI_Send((0b1010000000000000 | reg_number), 0x8000);
    }

    // write registers 96-111, setting the positive stimulation current magnitudes
    // to zero and the current trims to the center point
    // U flag controlled, U = 1
    for(reg_number = 0b01100000 ; reg_number <= 0b01101111; reg_number ++)
    {
        SPI_Send((0b1010000000000000 | reg_number), 0x8000);
    }

    // keep stimulation disabled

    // dummy command with M flag set to clear the compliance monitor
    // READ(255)
    // U = 1
    SPI_Send(0b1111000011111111, 0b0000000000000000); //1111 0000 1111 1111

    // clear the receive FIFO
    while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx[0]))
    {
        // do nothing
    }

    return 1;
}


// function to read register value and compare it with expected value
int read_check(uint32_t address, uint32_t expected_value)
{
    uint32_t DataRx_check[1]; // variable for receiving data

    SPI_Send((0b1100000000000000 | address), 0b0000000000000000);

    SPI_Send((0b1100000000000000 | address), 0b0000000000000000);
    // clear the receive FIFO
    while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx[0]))
    {
        // do nothing
    }

    SPI_Send((0b1100000000000000 | address), 0b0000000000000000);

    // collect the reading result
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_check[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_check[0]);

    if(! (DataRx_check[1] == 0x0000) ){
        return 0;
    }else{}
    if(! (DataRx_check[0] == expected_value) ){
        return 0;
    }else{}

    return 1;
}

// function to check register values after initialization
int headstage_init_check(void)
{
    volatile bool result;
    result = 1;

    // begin checking
    result &= read_check(255, 32);
    result &= read_check(32, 0x0000);
    result &= read_check(33, 0x0000);
    result &= read_check(38, 0xFFFF);
    result &= read_check(0, 0x00C7);
    // result &= read_check(1, 199);
    result &= read_check(2, 0x0040);
    result &= read_check(3, 0x0080);
    result &= read_check(4, 0x0016);
    result &= read_check(5, 0x0017);
    result &= read_check(6, 0x00A8);
    result &= read_check(7, 0x000A);
    result &= read_check(8, 0xFFFF);
    result &= read_check(10, 0x0000);
    result &= read_check(12, 0xFFFF);
    result &= read_check(34, 0x00E2);
    result &= read_check(35, 0x00AA);
    result &= read_check(36, 0x0080);
    result &= read_check(37, 0x4F00);
    result &= read_check(42, 0x0000);
    result &= read_check(44, 0x0000);
    result &= read_check(46, 0x0000);
    result &= read_check(48, 0x0000);

    result &= read_check(64, 0x8000);
    result &= read_check(65, 0x8000);
    result &= read_check(66, 0x8000);
    result &= read_check(67, 0x8000);
    result &= read_check(68, 0x8000);
    result &= read_check(69, 0x8000);
    result &= read_check(70, 0x8000);
    result &= read_check(71, 0x8000);
    result &= read_check(72, 0x8000);
    result &= read_check(73, 0x8000);
    result &= read_check(74, 0x8000);
    result &= read_check(75, 0x8000);
    result &= read_check(76, 0x8000);
    result &= read_check(77, 0x8000);
    result &= read_check(78, 0x8000);
    result &= read_check(79, 0x8000);

    result &= read_check(96, 0x8000);
    result &= read_check(97, 0x8000);
    result &= read_check(98, 0x8000);
    result &= read_check(99, 0x8000);
    result &= read_check(100, 0x8000);
    result &= read_check(101, 0x8000);
    result &= read_check(102, 0x8000);
    result &= read_check(103, 0x8000);
    result &= read_check(104, 0x8000);
    result &= read_check(105, 0x8000);
    result &= read_check(106, 0x8000);
    result &= read_check(107, 0x8000);
    result &= read_check(108, 0x8000);
    result &= read_check(109, 0x8000);
    result &= read_check(110, 0x8000);
    result &= read_check(111, 0x8000);

    /*
    int i;
    for(i=64; i<=79; i++){
        result &= read_check(i, 0x8000);
    }
    for(i=96; i<=111; i++){
        result &= read_check(i, 0x8000);
    }
    result &= read_check(32, 0x0000);
    result &= read_check(33, 0x0000);
     */

    return result;
}

void read_register(uint32_t address)
{
    uint32_t DataRx_check[1]; // variable for receiving data

    SPI_Send((0b1100000000000000 | address), 0b0000000000000000);

    SPI_Send((0b1100000000000000 | address), 0b0000000000000000);
    // clear the receive FIFO
    while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx[0]))
    {
        // do nothing
    }

    SPI_Send((0b1100000000000000 | address), 0b0000000000000000);

    // collect the reading result
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_check[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_check[0]);

    char filename[16];
    sprintf(filename, "%s%d", "%s", DataRx_check[0]);
    UARTprintf(filename);
    UARTprintf("\n");
}

int Bandwidth_set(uint32_t upper, uint32_t lower)
{
    switch(upper) {
    case 20000 :
        SPI_Send(0b1000000000000100, 0x0000 | (0<<6) | 8);
        SPI_Send(0b1000000000000101, 0x0000 | (0<<6) | 4);
        break;
    case 15000 :
        SPI_Send(0b1000000000000100, 0x0000 | (0<<6) | 11);
        SPI_Send(0b1000000000000101, 0x0000 | (0<<6) | 8);
        break;
    case 10000 :
        SPI_Send(0b1000000000000100, 0x0000 | (0<<6) | 17);
        SPI_Send(0b1000000000000101, 0x0000 | (0<<6) | 16);
        break;
    case 7500 :
        SPI_Send(0b1000000000000100, 0x0000 | (0<<6) | 22);
        SPI_Send(0b1000000000000101, 0x0000 | (0<<6) | 23);
        break;
    case 5000 :
        SPI_Send(0b1000000000000100, 0x0000 | (0<<6) | 33);
        SPI_Send(0b1000000000000101, 0x0000 | (0<<6) | 37);
        break;
    case 3000 :
        SPI_Send(0b1000000000000100, 0x0000 | (1<<6) | 3);
        SPI_Send(0b1000000000000101, 0x0000 | (1<<6) | 13);
        break;
    case 2500 :
        SPI_Send(0b1000000000000100, 0x0000 | (1<<6) | 13);
        SPI_Send(0b1000000000000101, 0x0000 | (1<<6) | 25);
        break;
    case 2000 :
        SPI_Send(0b1000000000000100, 0x0000 | (1<<6) | 27);
        SPI_Send(0b1000000000000101, 0x0000 | (1<<6) | 44);
        break;
    case 1500 :
        SPI_Send(0b1000000000000100, 0x0000 | (2<<6) | 1);
        SPI_Send(0b1000000000000101, 0x0000 | (2<<6) | 23);
        break;
    case 1000 :
        SPI_Send(0b1000000000000100, 0x0000 | (2<<6) | 46);
        SPI_Send(0b1000000000000101, 0x0000 | (3<<6) | 30);
        break;
    case 750 :
        SPI_Send(0b1000000000000100, 0x0000 | (3<<6) | 41);
        SPI_Send(0b1000000000000101, 0x0000 | (4<<6) | 36);
        break;
    case 500 :
        SPI_Send(0b1000000000000100, 0x0000 | (5<<6) | 30);
        SPI_Send(0b1000000000000101, 0x0000 | (6<<6) | 43);
        break;
    case 300 :
        SPI_Send(0b1000000000000100, 0x0000 | (9<<6) | 6);
        SPI_Send(0b1000000000000101, 0x0000 | (11<<6) | 2);
        break;
    case 250 :
        SPI_Send(0b1000000000000100, 0x0000 | (10<<6) | 42);
        SPI_Send(0b1000000000000101, 0x0000 | (13<<6) | 5);
        break;
    case 200 :
        SPI_Send(0b1000000000000100, 0x0000 | (13<<6) | 24);
        SPI_Send(0b1000000000000101, 0x0000 | (16<<6) | 7);
        break;
    case 150 :
        SPI_Send(0b1000000000000100, 0x0000 | (17<<6) | 44);
        SPI_Send(0b1000000000000101, 0x0000 | (21<<6) | 8);
        break;
    case 100 :
        SPI_Send(0b1000000000000100, 0x0000 | (26<<6) | 38);
        SPI_Send(0b1000000000000101, 0x0000 | (31<<6) | 5);
        break;
    default:
        return 0;
    }

    switch(lower) {
    case 1000 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 10);
        break;
    case 500 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 13);
        break;
    case 300 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 15);
        break;
    case 250 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 17);
        break;
    case 200 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 18);
        break;
    case 150 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 21);
        break;
    case 100 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 25);
        break;
    case 75 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 28);
        break;
    case 50 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 34);
        break;
    case 30 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 44);
        break;
    case 25 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 48);
        break;
    case 20 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 54);
        break;
    case 15 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 0<<7 | 62);
        break;
    case 10 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 1<<7 | 5);
        break;
    case 5 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 1<<7 | 40);
        break;
    case 3 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 2<<7 | 20);
        break;
    case 2 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 3<<7 | 8);
        break;
    case 1 :
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 6<<7 | 44);
        break;
    default:
        return 0;
    }

    return 1;
}

int register4_check(uint32_t upper)
{
    switch (upper){
    case 20000:
        if (read_check(4, 0x0000 | (0<<6) | 8) == 1){
            return 1;
        }else{
            return 0;
        }
    case 15000:
        if (read_check(4, 0x0000 | (0<<6) | 11) == 1){
            return 1;
        }else{
            return 0;
        }
    case 10000:
        if (read_check(4, 0x0000 | (0<<6) | 17) == 1){
            return 1;
        }else{
            return 0;
        }
    case 7500:
        if (read_check(4, 0x0000 | (0<<6) | 22) == 1){
            return 1;
        }else{
            return 0;
        }
    case 5000:
        if (read_check(4, 0x0000 | (0<<6) | 33) == 1){
            return 1;
        }else{
            read_register(4);
            return 0;
        }
    case 3000:
        if (read_check(4, 0x0000 | (1<<6) | 3) == 1){
            return 1;
        }else{
            return 0;
        }
    case 2500:
        if (read_check(4, 0x0000 | (1<<6) | 13) == 1){
            return 1;
        }else{
            return 0;
        }
    case 2000:
        if (read_check(4, 0x0000 | (1<<6) | 27) == 1){
            return 1;
        }else{
            return 0;
        }
    case 1500:
        if (read_check(4, 0x0000 | (2<<6) | 1) == 1){
            return 1;
        }else{
            return 0;
        }
    case 1000:
        if (read_check(4, 0x00AE) == 1){
            return 1;
        }else{
            return 0;
        }
    case 750:
        if (read_check(4, 0x0000 | (3<<6) | 41) == 1){
            return 1;
        }else{
            return 0;
        }
    case 500:
        if (read_check(4, 0x0000 | (5<<6) | 30) == 1){
            return 1;
        }else{
            return 0;
        }
    case 300:
        if (read_check(4, 0x0000 | (9<<6) | 6) == 1){
            return 1;
        }else{
            return 0;
        }
    case 250:
        if (read_check(4, 0x0000 | (10<<6) | 42) == 1){
            return 1;
        }else{
            return 0;
        }
    case 200:
        if (read_check(4, 0x0000 | (13<<6) | 24) == 1){
            return 1;
        }else{
            return 0;
        }
    case 150:
        if (read_check(4, 0x0000 | (17<<6) | 44) == 1){
            return 1;
        }else{
            return 0;
        }
    case 100:
        if (read_check(4, 0x0000 | (26<<6) | 38) == 1){
            return 1;
        }else{
            return 0;
        }
    default:
        return 0;
    }
}


int register5_check(uint32_t upper)
{
    switch (upper){
    case 20000:
        if (read_check(5, 0x0000 | (0<<6) | 4) == 1){
            return 1;
        }else{
            return 0;
        }
    case 15000:
        if (read_check(5, 0x0000 | (0<<6) | 8) == 1){
            return 1;
        }else{
            return 0;
        }
    case 10000:
        if (read_check(5, 0x0000 | (0<<6) | 16) == 1){
            return 1;
        }else{
            return 0;
        }
    case 7500:
        if (read_check(5, 0x0000 | (0<<6) | 23) == 1){
            return 1;
        }else{
            return 0;
        }
    case 5000:
        if (read_check(5, 0x0000 | (0<<6) | 37) == 1){
            return 1;
        }else{
            return 0;
        }
    case 3000:
        if (read_check(5, 0x0000 | (1<<6) | 13) == 1){
            return 1;
        }else{
            return 0;
        }
    case 2500:
        if (read_check(5, 0x0000 | (1<<6) | 25) == 1){
            return 1;
        }else{
            return 0;
        }
    case 2000:
        if (read_check(5, 0x0000 | (1<<6) | 44) == 1){
            return 1;
        }else{
            return 0;
        }
    case 1500:
        if (read_check(5, 0x0000 | (2<<6) | 23) == 1){
            return 1;
        }else{
            return 0;
        }
    case 1000:
        if (read_check(5, 0x00DE) == 1){
            return 1;
        }else{
            return 0;
        }
    case 750:
        if (read_check(5, 0x0000 | (4<<6) | 36) == 1){
            return 1;
        }else{
            return 0;
        }
    case 500:
        if (read_check(5, 0x0000 | (6<<6) | 43) == 1){
            return 1;
        }else{
            return 0;
        }
    case 300:
        if (read_check(5, 0x0000 | (11<<6) | 2) == 1){
            return 1;
        }else{
            return 0;
        }
    case 250:
        if (read_check(5, 0x0000 | (13<<6) | 5) == 1){
            return 1;
        }else{
            return 0;
        }
    case 200:
        if (read_check(5, 0x0000 | (16<<6) | 7) == 1){
            return 1;
        }else{
            return 0;
        }
    case 150:
        if (read_check(5, 0x0000 | (21<<6) | 8) == 1){
            return 1;
        }else{
            return 0;
        }
    case 100:
        if (read_check(5, 0x0000 | (31<<6) | 5) == 1){
            return 1;
        }else{
            return 0;
        }
    default:
        return 0;
    }
}


int register6_check(uint32_t lower)
{
    switch (lower){
    case 1000:
        if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 10) == 1){
            return 1;
        }else{
            return 0;
        }
    case 500:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 13) == 1){
                return 1;
            }else{
                return 0;
            }
    case 300:
            if (read_check(6, 0x000F) == 1){
                return 1;
            }else{
                return 0;
            }
    case 250:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 17) == 1){
                return 1;
            }else{
                return 0;
            }
    case 200:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 18) == 1){
                return 1;
            }else{
                return 0;
            }
    case 150:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 21) == 1){
                return 1;
            }else{
                return 0;
            }
    case 100:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 25) == 1){
                return 1;
            }else{
                return 0;
            }
    case 75:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 28) == 1){
                return 1;
            }else{
                return 0;
            }
    case 50:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 34) == 1){
                return 1;
            }else{
                return 0;
            }
    case 30:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 44) == 1){
                return 1;
            }else{
                return 0;
            }
    case 25:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 48) == 1){
                return 1;
            }else{
                return 0;
            }
    case 20:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 54) == 1){
                return 1;
            }else{
                return 0;
            }
    case 15:
            if (read_check(6, 0x0000 | 0<<13 | 0<<7 | 62) == 1){
                return 1;
            }else{
                return 0;
            }
    case 10:
            if (read_check(6, 0x0000 | 0<<13 | 1<<7 | 5) == 1){
                return 1;
            }else{
                return 0;
            }
    case 5:
            if (read_check(6, 0x0000 | 0<<13 | 1<<7 | 40) == 1){
                return 1;
            }else{
                return 0;
            }
    case 3:
            if (read_check(6, 0x0000 | 0<<13 | 2<<7 | 20) == 1){
                return 1;
            }else{
                return 0;
            }
    case 2:
            if (read_check(6, 0x0000 | 0<<13 | 3<<7 | 8) == 1){
                return 1;
            }else{
                return 0;
            }
    case 1:
            if (read_check(6, 0x0000 | 0<<13 | 6<<7 | 44) == 1){
                return 1;
            }else{
                return 0;
            }
    default:
        return 0;
    }
}

// function to initialize SSI module
void SSI_init(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    ROM_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    ROM_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
    ROM_GPIOPinConfigure(GPIO_PA4_SSI0RX);
    ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);


    // port A6 is used for FSS output
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);
    ROM_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0b1000000);


    ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);

    ROM_SSIConfigSetExpClk(SSI0_BASE, ROM_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 16);

    ROM_SSIEnable(SSI0_BASE);

    while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx[0]))
    {
        // clear the receive FIFO
    }
}

// main function
int main(void)
{

    // buffer for read data from SD Card
    TCHAR read_buffer[32];
    TCHAR *command;
    TCHAR *value;

    // counter for loops when writing to SD card
    uint32_t store_count;
    uint32_t i = 0;

    // data read from SD card for configuration
    uint32_t read_value;
    uint32_t sampling_frequency;
    uint32_t file_seconds;

    uint32_t cutoff_upper;
    uint32_t cutoff_lower;

    char def_filename[16];
    char filename[16];

    // counter for current file
    uint32_t file_counter = 0;

    n = 0x00;

    channel_number = 8; // this should be set by user input

    // Set the clocking to run from the PLL at 50MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

//#ifdef DEBUG
//    uart_init();
//#endif

    //I changed this cuz I don't want to change everything else
    uart_init();

//    // Initialise USBCDC for VCP.
//    USBInit();
//    UARTprintf("hello\n");
    // Initialise USBCDC for VCP.
    // Enable and configure the GPIO port for the LED operation.
    // EVK Board
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, LED_0 | LED_1 | LED_2 | LED_3 );
    USBInit();
     // Update the message in the buffer
     uint32_t ui32Bytes;
     uint8_t ui8RxBuffer[MAX_PLOAD];
     strcpy(ui8RxBuffer, "bl\r\n");
     ui32Bytes = strlen(ui8RxBuffer);

     //USBBufferWrite(&TxBuffer, ui8RxBuffer, ui32Bytes);
     //UARTprintf("hello\n");

#ifdef DEBUG
    UARTprintf("OK, let's begin!\n");
#endif

    /***********************************************************/
    /* SD Card configuration */
    /***********************************************************/
    // configure the systick timer for a 100Hz interrupt. required by the FatFs driver.
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 100);
    SysTickIntRegister(SysTickHandler);
    ROM_SysTickEnable();
    ROM_SysTickIntEnable();

    // Delay a little for SD card to initialise
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3);

    // mount the SD Card using logical drive 0.
    //rc = f_mount(0, &fatfs);
    //*****for some reason if I remote this line of code, it stops working*******/
    rc = FR_OK;

                sampling_frequency = SAMPLING_FREQ;
                buffer_size = BUFFER_SIZE;
#ifdef DEBUG
                UARTprintf("adjusted buffer_size:%d\n", buffer_size);
#endif

                file_seconds = FILE_SECONDS;
                char temp[16] = "0123_";
                strcpy(def_filename, temp);

                cutoff_upper = CUTOFF_UPPER;

                cutoff_lower = CUTOFF_LOWER;

        //***********without a delay, doesn't work!********
        ROM_SysCtlDelay(ROM_SysCtlClockGet()/3);


    /***********************************************************/
    /* set device configuration*/
    /***********************************************************/

    // set up buffers
    bufferA = (uint16_t *)malloc(buffer_size*sizeof(uint16_t));
    bufferB = (uint16_t *)malloc(buffer_size*sizeof(uint16_t));

    // initial config
    buffer_mode = BUFFER_A; // first start from buffer A
    storage_on = false; // currently write to SD card is disabled
    bufferA_empty = true; // buffer is empty
    bufferB_empty = true;   // buffer is empty

    // number of times complete buffer should be written to the SD card
    // to match the duration specified by file_minutes and
    // using 16-bit data (12-bit ADC resolution)
    store_count = (file_seconds * sampling_frequency) / buffer_size;
#ifdef DEBUG
    UARTprintf("Store count in each file: %d\n", store_count);
#endif

    SSI_init();
    /***********************************************************/
    /* Initialize the headstage */
    /***********************************************************/
    if(headstage_init() == 0){
#ifdef DEBUG
        UARTprintf("Headstage cannot be initialized. Bye!\n");
#endif
        return 0;
    }else{
#ifdef DEBUG
        UARTprintf("Headstage successfully initialized!\n");
#endif
    }

    /***********************************************************/
    /* Check the register values */
    /***********************************************************/
    if(headstage_init_check() == 1){
#ifdef DEBUG
        UARTprintf("Register values checked!\n");
#endif
    }else{
#ifdef DEBUG
        UARTprintf("Register values are not as expected. Bye!\n");
#endif
        return 0;
    }

    /***********************************************************/
    /* Check the register 1 specifically */
    /***********************************************************/
    if(read_check(1, 0x051A) == 1){
#ifdef DEBUG
        UARTprintf("Register 1 checked!\n");
#endif
    }else{
#ifdef DEBUG
        UARTprintf("Register 1 value is not as expected, it's:\n");
        read_register(1);
#endif
        return 0;
    }

    // re-write the register 0, according to desired sampling rate
    SPI_Send(0b1000000000000000, 0x0828); //1000 0000 0000 0000, 0000 1000 0010 1000
    // check register value
    if(read_check(0, 0b0000100000101000) == 1){
#ifdef DEBUG
        UARTprintf("Sampling frequency set to be smaller than 120k!\n");
#endif
    }else{
#ifdef DEBUG
        UARTprintf("Register 0 value is not as expected, it's:\n");
        read_register(0);
#endif
        return 0;
    }

    // re-write register 4,5,6
    if (Bandwidth_set(cutoff_upper, cutoff_lower) == 1){
#ifdef DEBUG
        UARTprintf("Bandwidth set!\n");
#endif
    }else{
        UARTprintf("Unable to set bandwidth!\n");
    }

    if (register4_check(cutoff_upper) == 1){
#ifdef DEBUG
        UARTprintf("Register 4 checked!\n");
#endif
    }else{
#ifdef DEBUG
        UARTprintf("Unable to set register 4, bye!\n");
#endif
        return 0;
    }

    if (register5_check(cutoff_upper) == 1){
#ifdef DEBUG
        UARTprintf("Register 5 checked!\n");
#endif
    }else{
#ifdef DEBUG
        UARTprintf("Unable to set register 5, bye!\n");
#endif
        return 0;
    }

    if (register6_check(cutoff_lower) == 1){
#ifdef DEBUG
        UARTprintf("Register 6 checked!\n");
#endif
    }else{
#ifdef DEBUG
        UARTprintf("Unable to set register 6, bye!\n");
#endif
        return 0;
    }

    /*
    while(ROM_SSIDataGetNonBlocking(SSI0_BASE, &pui32DataRx[0]))
    {
        // clear the receive FIFO
    }*/

    /***********************************************************/
    /* Enable global interrupt */
    /***********************************************************/
    ROM_IntMasterEnable();

    // Enable timer0, so sampling begins
    timer0_init(sampling_frequency);

    /***********************************************************/
    /* Open first file to write */
    /***********************************************************/
//    rc = f_open(&fil, filename, FA_CREATE_ALWAYS | FA_WRITE);
//    if(rc != FR_OK)
//    {
//#ifdef DEBUG
//        UARTprintf("Cannot open file for writing data. Bye!\n");
//#endif
//        return 0;
//    }
//    else
//    {
//#ifdef DEBUG
//        UARTprintf("Now writing to file: %s\n", filename);
//#endif
//    }

    /***********************************************************/
    /* Main application loop */
    /***********************************************************/
    while(1){
        if(storage_on) { // check if write to SD card is enabled

            if(buffer_mode == BUFFER_A) { // if A is currently being used, transfer from B

#ifdef DEBUG
                UARTprintf("i: %d\n", i);
#endif

                bufferB_empty = true;
                //ROM_SysCtlDelay(5);
                storage_on = false;
            }
            else if(buffer_mode == BUFFER_B) { // if B is currently being used, transfer from A.

                i = i + 1;
                //bufferA_empty = true;
                storage_on = false;
            }
        }
        else {
            ROM_SysCtlSleep();
        }
    }
}

volatile uint32_t ui32Bytes = 0;
volatile uint8_t ui8RxBuffer[MAX_PLOAD];
volatile uint8_t byte1, byte2;

// Handler for timer0
void Timer0IntHandler(void) {

    // clear the timer interrupt
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    uint32_t ADCValue[1];

    // do one convert
    SPI_Send(channel_number, 0);

    // read conversion result from FIFO
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &ADCValue[0]); //high gain amplifier result
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &ADCValue[1]);
    //ADCValue is 2 bytes, make it 2 one byte val
    byte1 = ADCValue[0] & 0xFF;
    byte2 = (ADCValue[0] >> 8) & 0xFF;

    //UARTprintf("%d\n", ADCValue[0]);
    // Update the message in the buffer
    ui8RxBuffer[ui32Bytes] = byte1;
    ui32Bytes++;
    ui8RxBuffer[ui32Bytes] = byte2;
    ui32Bytes++;
//    ui8RxBuffer[ui32Bytes] = 0xFF;
//    ui32Bytes++;
//    ui8RxBuffer[ui32Bytes] = 0xFF;
//    ui32Bytes++;
    if(ui32Bytes == 2){
        USBBufferWrite(&TxBuffer, ui8RxBuffer, ui32Bytes);
        ui32Bytes = 0;
    }
}
void RxDataHandler(){}


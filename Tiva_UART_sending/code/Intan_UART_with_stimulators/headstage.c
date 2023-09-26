// function to initialize the headstage using SPI


#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "main.h"

uint32_t pui32DataRx[1];
uint32_t reg34_val;
uint32_t reg35_val;

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

    // write register 34, set the full scale range to...
    //keep track of init val
    //uint32_t reg34_val = 0b0000010100011001;   //51uA
    reg34_val = 0b0000000000001111;     //2.55mA
    SPI_Send(0b1010000000100010, reg34_val); // 0 00 001010 0011001

    // check the received data
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[1]);
    ROM_SSIDataGetNonBlocking(SSI0_BASE, &DataRx_init[0]);
    if(! (DataRx_init[1] == 0xFFFF) ){
        return 0;
    }else{}
    if(! (DataRx_init[0] == 0x0000) ){
        return 0;
    }else{}

    // write register 35, set the full scale range to...
    //uint32_t reg35_val = 0b0000000010001000;  //51uA
    reg35_val = 0b0000000011111111;

    SPI_Send(0b1010000000100011, reg35_val); // 00000000 1000 1000

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
    if(! (DataRx_init[0] == reg34_val) ){
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
    if(! (DataRx_init[0] == reg35_val) ){
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

    // write register 44, set all stimulators to positive polarity
    // U flag controlled, U = 1
    SPI_Send(0b1010000000101100, 0b1111111111111111); // 1111 1111 1111 1111

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
    if(! (DataRx_init[0] == 0b1111111111111111) ){
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
    result &= read_check(34, reg34_val);
    result &= read_check(35, reg35_val);
    result &= read_check(36, 0x0080);
    result &= read_check(37, 0x4F00);
    result &= read_check(42, 0x0000);
    result &= read_check(44, 0b1111111111111111);
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

    return result;
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
    case 53:
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 17<<7 | 35);
        break;
    case 52:
        SPI_Send(0b1000000000000110, 0x0000 | 0<<13 | 54<<7 | 56);
        break;
    case 51:
        SPI_Send(0b1000000000000110, 0x0000 | 1<<13 | 60<<7 | 16);
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


int Frequency_set(uint32_t frequency)
{
    if (frequency <= 120000){
        SPI_Send(0b1000000000000000, 0b0 | (32<<6) | 40);
    }
    else if ((frequency > 120000) && (frequency <= 140000)){
        SPI_Send(0b1000000000000000, 0b0 | (16<<6) | 40);
    }
    else if ((frequency > 140000) && (frequency <= 175000)){
        SPI_Send(0b1000000000000000, 0b0 | (8<<6) | 40);
    }
    else if ((frequency > 175000) && (frequency <= 220000)){
        SPI_Send(0b1000000000000000, 0b0 | (8<<6) | 32);
    }
    else if ((frequency > 220000) && (frequency <= 280000)){
        SPI_Send(0b1000000000000000, 0b0 | (8<<6) | 26);
    }
    else if ((frequency > 280000) && (frequency <= 350000)){
        SPI_Send(0b1000000000000000, 0b0 | (4<<6) | 18);
    }
    else if ((frequency > 350000) && (frequency <= 440000)){
        SPI_Send(0b1000000000000000, 0b0 | (3<<6) | 16);
    }
    else if (frequency >= 440000){
        SPI_Send(0b1000000000000000, 0b0 | (3<<6) | 7);
    }
    else{
        return 0;
    }

    return 1;
}

int register0_check(uint32_t frequency)
{
    if (frequency <= 120000){
        if (read_check(0, 0x0828) == 1){
            return 1;
        }else{
            return 0;
        }
    }
    else if ((frequency > 120000) && (frequency <= 140000)){
        if (read_check(0, 0x0428) == 1){
            return 1;
        }else{
            return 0;
        }
    }
    else if ((frequency > 140000) && (frequency <= 175000)){
        if (read_check(0, 0x0228) == 1){
            return 1;
        }else{
            return 0;
        }
    }
    else if ((frequency > 175000) && (frequency <= 220000)){
        if (read_check(0, 0x0220) == 1){
            return 1;
        }else{
            return 0;
        }
    }
    else if ((frequency > 220000) && (frequency <= 280000)){
        if (read_check(0, 0x021A) == 1){
            return 1;
        }else{
            return 0;
        }
    }
    else if ((frequency > 280000) && (frequency <= 350000)){
        if (read_check(0, 0x0112) == 1){
            return 1;
        }else{
            return 0;
        }
    }
    else if ((frequency > 350000) && (frequency <= 440000)){
        if (read_check(0, 0x00D0) == 1){
            return 1;
        }else{
            return 0;
        }
    }
    else if (frequency >= 440000){
        if (read_check(0, 0x00C7) == 1){
            return 1;
        }else{
            return 0;
        }
    }
    else{
        return 0;
    }
}



#include "ads1299.h"
#include "spi.h"

//#define 18TCLKS_PLACEHOLDER 300000
//#define 4TCLKS_PLACEHOLDER 30000

void ADS1299_SPI_Init(void)
{
    uint32_t ui32ResidualData;
    // enable SSI3 and GPIOD peripherals
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // CE and IRQ are on Port B

    // Configure GPIO pins for special functions except CSN, CE and IRQ which are under software control
    GPIOPinConfigure(GPIO_PD0_SSI3CLK);
    GPIOPinConfigure(GPIO_PD2_SSI3RX);
    GPIOPinConfigure(GPIO_PD3_SSI3TX);
    ROM_GPIOPinTypeSSI(SPI_PORTD_BASE, SCK | MISO | MOSI);

    // Configure GPIO pins for CSN
    ROM_GPIOPinTypeGPIOOutput(CS_BASE, CSN);

    // Configure and enable SSI port
    // Use internal 16Mhz RC oscillator as SSI clock source
    ROM_SSIClockSourceSet(SPI_BASE, SSI_CLOCK_PIOSC);
    ROM_SSIConfigSetExpClk(SPI_BASE, ROM_SysCtlClockGet(), SSI_FRF_MOTO_MODE_1, SSI_MODE_MASTER, 2000000, 8);
    ROM_SSIEnable(SPI_BASE);

    // Read any residual data on the SSI port to clear buffer
    while(SSIDataGetNonBlocking(SPI_BASE, &ui32ResidualData))
    {

    }
}

// Initialize ADS1299 for ReadReg Command
void ADS1299_Init_ReadReg(void)
{
    // Send Reset Command
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ADS1299_RESET_CMD);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);
    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);

    // Wait 18 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/300000);

    // Send SDATAC Command
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ADS1299_SDATAC_CMD);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);
    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);

    // Wait 4 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);

}

// Initialize ADS1299 for ReadReg Command
void ADS1299_Init(void)
{
    // Send Reset Command
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ADS1299_RESET_CMD);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);
    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);

    // Wait 18 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/300000);

    // Send SDATAC Command
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ADS1299_SDATAC_CMD);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);
    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);

    // Wait 4 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);

    // Write Config3 0xE0 to use internal reference
    ADS1299_WREG(ADS1299_CONFIG3, 0xE0);

    // Wait 4 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);

    // Write Config1 0x96 to Set Device for DR = fMOD/4096
    ADS1299_WREG(ADS1299_CONFIG1, 0x96);

    // Wait 4 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);

    // Write Config2 0xC0
    ADS1299_WREG(ADS1299_CONFIG2, 0xC0);

    // Wait 4 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);

    // Write CHnSET 0x01 to set all channels to input short
    ADS1299_WREG(ADS1299_CH1SET, 0x01);
    ADS1299_WREG(ADS1299_CH2SET, 0x01);
    ADS1299_WREG(ADS1299_CH3SET, 0x01);
    ADS1299_WREG(ADS1299_CH4SET, 0x01);
    ADS1299_WREG(ADS1299_CH5SET, 0x01);
    ADS1299_WREG(ADS1299_CH6SET, 0x01);
    ADS1299_WREG(ADS1299_CH7SET, 0x01);
    ADS1299_WREG(ADS1299_CH8SET, 0x01);

    // Wait 4 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);

    // Send START Command
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ADS1299_START_CMD);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);
    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);

    // Wait 4 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);

    // Send RDATAC Command
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ADS1299_RDATAC_CMD);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);
    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);




}

// function to write a byte to the SPI port
void ADS1299_SPI_Write(uint32_t ui32Register, uint32_t ui32Value)
{
    uint32_t ui32ResidualData;
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ui32Register);
    SPIDataRead();
    SPIDataWrite(ui32Value);
    SPIDataRead();
    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);
}

uint32_t ADS1299_RREG(uint32_t ui32Register)
{
    // Send SDATAC Command
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ADS1299_SDATAC_CMD);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);
    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);

    // Wait 4 tclks
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/3000000);

    uint32_t readVal;

    // ReadReg format 001r rrrr where r is starting reg address
    ui32Register = ui32Register | 0x20;
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ui32Register);

    // Read two registers (000n nnnn where n = size - 1)
    SPIDataWrite(0x01);

    // Provide clock signal
    SPIDataWrite(0xFF);
    SPIDataWrite(0xFF);

    // Wait some clocks for spi read back
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/300000);

    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);
    return readVal;
}

void ADS1299_START()
{
    uint8_t tx = ADS1299_START_CMD;

    ROM_GPIOPinWrite(CS_BASE, CSN, 0);

    SPIDataWrite(ADS1299_START_CMD);
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/30000);

    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);

}

void ADS1299_WREG(uint32_t ui32Register, uint32_t ui32Data)
{
    uint32_t readVal;

    // ReadReg format 001r rrrr where r is starting reg address
    ui32Register = ui32Register | 0x40;
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ui32Register);

    // Write one register (000n nnnn where n = size - 1)
    SPIDataWrite(0x00);

    // Write data
    SPIDataWrite(ui32Data);

    // Wait some clocks for spi write
    ROM_SysCtlDelay(ROM_SysCtlClockGet()/300000);

    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);
}

void ADS1299_RDATA()
{
    int i;

    // Send RDATA Command
    ROM_GPIOPinWrite(CS_BASE, CSN, 0);
    SPIDataWrite(ADS1299_RDATA_CMD);

    // Provide 24 bits status reg + 8 * 24 btis readings
    for (i = 0; i < 27; i++)
    {
        SPIDataWrite(0x00);
    }

    ROM_SysCtlDelay(ROM_SysCtlClockGet()/100000);

    ROM_GPIOPinWrite(CS_BASE, CSN, CSN);

}

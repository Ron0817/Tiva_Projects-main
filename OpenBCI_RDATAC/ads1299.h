
#include  "ads1299_reg.h"
#include "spi.h"

//System CMD
//wake-up from standby mode
#define ADS1299_WAKEUP_CMD 0x02
//enter standby mode
#define ADS1299_STANDBY_CMD 0x04
//reset the device
#define ADS1299_RESET_CMD 0x06
//start and restart(synchronize) conversions
#define ADS1299_START_CMD 0x08
//stop conversion
#define ADS1299_STOP_CMD 0x0A

//Data Read CMD
//default mode, enable read data continuous mode
#define ADS1299_RDATAC_CMD 0x10
//stop read ata continuously mode
#define ADS1299_SDATAC_CMD 0x11
//read tat by command
#define ADS1299_RDATA_CMD 0x12

//Register Read CMD
//start is the starting register address for read or write commands
//size is the number of reg want to read or write
#define ADS1299_RREG_CMD(start,size) {0b00100000 | start, size}
#define ADS1299_WREG_CMD(start,size) {0b01000000 | start, size}

void ADS1299_SPI_Init(void);

// TODOs: To be deleted
void ADS1299_Init_ReadReg(void);

void ADS1299_init(void);

// TODOs: Change uint32_t to uint8_t
void ADS1299_SPI_Write(uint32_t ui32Register, uint32_t ui32Value);

uint32_t ADS1299_RREG(uint32_t ui32Register);

void ADS1299_WREG(uint32_t ui32Register, uint32_t ui32Data);

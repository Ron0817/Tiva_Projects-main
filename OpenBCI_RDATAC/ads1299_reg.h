//Read Only ID Register
#define ADS1299_ID              0x00

//Global Setting Across Channels
#define ADS1299_CONFIG1         0x01
#define ADS1299_CONFIG2         0x02
#define ADS1299_CONFIG3         0x03
#define ADS1299_LOFF            0x04

//channel-Specific Setting
#define ADS1299_CH1SET         0x05
#define ADS1299_CH2SET         0x06
#define ADS1299_CH3SET         0x07
#define ADS1299_CH4SET         0x08
//only avaliable in ADS1299-6, ADS1299
#define ADS1299_CH5SET         0x09
#define ADS1299_CH6SET         0x0A
//only avaliable in ADS1299
#define ADS1299_CH7SET         0x0B
#define ADS1299_CH8SET         0x0C
#define ADS1299_BIAS_SENSP     0x0D
#define ADS1299_BIAS_SENSN     0x0E
#define ADS1299_LOFF_SENSP     0x0F
#define ADS1299_LOFF_SENSP     0x10
#define ADS1299_LOFF_FLIP      0x11

//Lead-Off Status Register(read only)
#define ADS1299_LOFF_STATP     0x12
#define ADS1299_LOFF_STATN     0x13

//GPIO AND OTHER Registers
#define ADS1299_GPIO      0x14
#define ADS1299_MISC1     0x15
#define ADS1299_MISC2     0x16
#define ADS1299_CONFIG4     0x17
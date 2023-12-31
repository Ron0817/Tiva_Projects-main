/*
* icm20948.c
*
*  Created on: Dec 26, 2020
*      Author: mokhwasomssi
*
*  Modified on: Oct 2, 2023
*      Author: Rongxuan Du
*      Content: Adapted to communicate with Tiva Tm4c123gxl board
*/

#include "icm20948.h"

/* Static Functions */
static float gyro_scale_factor;
static float accel_scale_factor;

void     cs_high();
void     cs_low();
static void     select_user_bank(int ub);
//static uint32_t* read_multiple_icm20948_reg(int ub, uint8_t reg, uint8_t len);
//static void     write_multiple_icm20948_reg(int ub, uint8_t reg, uint8_t* val, uint8_t len);

/* Main Functions */
void icm20948_init()
{
    icm20948_device_reset();

    icm20948_wakeup();

    icm20948_clock_source(1);
    icm20948_odr_align_enable();

    icm20948_spi_slave_enable();

    icm20948_gyro_low_pass_filter(0);
    icm20948_accel_low_pass_filter(0);

    icm20948_gyro_sample_rate_divider(0);
    icm20948_accel_sample_rate_divider(0);

    icm20948_gyro_calibration();
    icm20948_accel_calibration();

    icm20948_gyro_full_scale_select(_1000dps);
    icm20948_accel_full_scale_select(_8g);
}

void icm20948_gyro_read(axises* data)
{
//    uint8_t* temp = read_multiple_icm20948_reg(0, B0_GYRO_XOUT_H, 6);
    int32_t temp[6] = {0};
    temp[0] = read_single_icm20948_reg(0, B0_GYRO_XOUT_H);
    temp[1] = read_single_icm20948_reg(0, B0_GYRO_XOUT_L);
    temp[2] = read_single_icm20948_reg(0, B0_GYRO_YOUT_H);
    temp[3] = read_single_icm20948_reg(0, B0_GYRO_YOUT_L);
    temp[4] = read_single_icm20948_reg(0, B0_GYRO_ZOUT_H);
    temp[5] = read_single_icm20948_reg(0, B0_GYRO_ZOUT_L);

    data->x = (int16_t)(temp[0] << 8 | temp[1]);
    data->y = (int16_t)(temp[2] << 8 | temp[3]);
    data->z = (int16_t)(temp[4] << 8 | temp[5]);

}

void icm20948_accel_read(axises* data)
{
//    uint8_t* temp = read_multiple_icm20948_reg(0, B0_ACCEL_XOUT_H, 6);
    int32_t temp[6] = {0};
    temp[0] = read_single_icm20948_reg(0, B0_ACCEL_XOUT_H);
    temp[1] = read_single_icm20948_reg(0, B0_ACCEL_XOUT_L);
    temp[2] = read_single_icm20948_reg(0, B0_ACCEL_YOUT_H);
    temp[3] = read_single_icm20948_reg(0, B0_ACCEL_YOUT_L);
    temp[4] = read_single_icm20948_reg(0, B0_ACCEL_ZOUT_H);
    temp[5] = read_single_icm20948_reg(0, B0_ACCEL_ZOUT_L);

    data->x = (int16_t)(temp[0] << 8 | temp[1]);
    data->y = (int16_t)(temp[2] << 8 | temp[3]);
    data->z = (int16_t)(temp[4] << 8 | temp[5]);

    //    data->z = (int16_t)(temp[4] << 8 | temp[5]) + accel_scale_factor;
    // Add scale factor because calibraiton function offset gravity acceleration. - Really?
}

void icm20948_gyro_read_dps(axises* data)
{
    icm20948_gyro_read(data);

    data->x /= gyro_scale_factor;
    data->y /= gyro_scale_factor;
    data->z /= gyro_scale_factor;
}

void icm20948_accel_read_g(axises* data)
{
    icm20948_accel_read(data);

    // if not divide the reading would be undetectable
    data->x /= (accel_scale_factor / 128);
    data->y /= (accel_scale_factor / 128);
    data->z /= (accel_scale_factor / 128);
}

/* Sub Functions */
uint32_t icm20948_who_am_i()
{
    return (uint32_t) (read_single_icm20948_reg(0, B0_WHO_AM_I));
}

void icm20948_device_reset()
{
    // Reset sleep mode. Set clock source to auto-select
    write_single_icm20948_reg(0, B0_PWR_MGMT_1, 0x80 | 0x41);
    ROM_SysCtlDelay(SysCtlClockGet()/10);

    UARTprintf("CHECK: B0_PWR_MGMT_1 = %x\n", ICM_SPI_Read(B0_PWR_MGMT_1));
}

void icm20948_wakeup()
{
    uint32_t new_val = 0x01;

    //  Must have UARTprintf first. Why?
    UARTprintf("wake up val is %x\n", new_val);

    //  Set CLKSEL 0x01 to allow full gyro performance
    ICM_SPI_Write(B0_PWR_MGMT_1, new_val);

    // Confirm correct val. To be deleted
    new_val = ICM_SPI_Read(B0_PWR_MGMT_1);
    UARTprintf("CHECK: WAKE UP NEW VAL = 0x01? -> 0x%x\n", new_val);

    ROM_SysCtlDelay(SysCtlClockGet()/10);
}

//void icm20948_sleep()
//{
//    uint8_t new_val = read_single_icm20948_reg(ub_0, B0_PWR_MGMT_1);
//    new_val |= 0x40;
//
//    write_single_icm20948_reg(ub_0, B0_PWR_MGMT_1, new_val);
//    HAL_Delay(100);
//}

void icm20948_spi_slave_enable()
{
    uint32_t new_val = 0x10;

//    UARTprintf("*********************spi_slave_enable val = %x\n", read_single_icm20948_reg(0, B0_USER_CTRL));

    // Set I2C_IF_DIS bit = 1 to allow SPI only
    ICM_SPI_Write(0x7F, 0);
    ICM_SPI_Write(B0_USER_CTRL, new_val);
    UARTprintf("CHECK: new spi_slave_enable val = 0x10? -> 0x%x\n", ICM_SPI_Read(B0_USER_CTRL));
}

//void icm20948_i2c_master_reset()
//{
//    uint8_t new_val = read_single_icm20948_reg(ub_0, B0_USER_CTRL);
//    new_val |= 0x02;
//
//    write_single_icm20948_reg(ub_0, B0_USER_CTRL, new_val);
//}
//
//void icm20948_i2c_master_enable()
//{
//    uint8_t new_val = read_single_icm20948_reg(ub_0, B0_USER_CTRL);
//    new_val |= 0x20;
//
//    write_single_icm20948_reg(ub_0, B0_USER_CTRL, new_val);
//    HAL_Delay(100);
//}
//
//void icm20948_i2c_master_clk_frq(uint8_t config)
//{
//    uint8_t new_val = read_single_icm20948_reg(ub_3, B3_I2C_MST_CTRL);
//    new_val |= config;
//
//    write_single_icm20948_reg(ub_3, B3_I2C_MST_CTRL, new_val);
//}

void icm20948_clock_source(uint32_t source)
{
    uint8_t new_val = read_single_icm20948_reg(0, B0_PWR_MGMT_1);
    new_val |= source;
    write_single_icm20948_reg(0, B0_PWR_MGMT_1, new_val);

    new_val = read_single_icm20948_reg(0, B0_PWR_MGMT_1);
    UARTprintf("CHECK CLKSET source = 0x01? -> 0x%x\n", new_val);
}

void icm20948_odr_align_enable()
{
   write_single_icm20948_reg(2, B2_ODR_ALIGN_EN, 0x01);
   UARTprintf("CHECK: new ord_align_en val = 0x01? ->  %x\n", ICM_SPI_Read(0x09));
}

void icm20948_gyro_low_pass_filter(uint32_t config)
{
    uint8_t new_val = read_single_icm20948_reg(2, B2_GYRO_CONFIG_1);

    new_val |= config << 3;
    write_single_icm20948_reg(2, B2_GYRO_CONFIG_1, new_val);
    UARTprintf("CHECK: gyro_config_1 reg val  = 0x01? ->  %x\n", ICM_SPI_Read(B2_GYRO_CONFIG_1));
}

void icm20948_accel_low_pass_filter(uint32_t config)
{
    uint32_t new_val = read_single_icm20948_reg(2, B2_ACCEL_CONFIG);
    new_val |= config << 3;

    write_single_icm20948_reg(2, B2_ACCEL_CONFIG, new_val);
}

void icm20948_gyro_sample_rate_divider(uint32_t divider)
{
    write_single_icm20948_reg(2, B2_GYRO_SMPLRT_DIV, divider);
}

void icm20948_accel_sample_rate_divider(uint32_t divider)
{
    uint8_t divider_1 = (uint8_t)(divider >> 8);
    uint8_t divider_2 = (uint8_t)(0x0F & divider);

    write_single_icm20948_reg(2, B2_ACCEL_SMPLRT_DIV_1, divider_1);
    write_single_icm20948_reg(2, B2_ACCEL_SMPLRT_DIV_2, divider_2);
}

void icm20948_gyro_calibration()
{
    int i;
    axises temp;
    int32_t gyro_bias[3] = {0};
    uint8_t gyro_offset[6] = {0};

    for(i = 0; i < 100; i++)
    {
        icm20948_gyro_read(&temp);
        gyro_bias[0] += temp.x;
        gyro_bias[1] += temp.y;
        gyro_bias[2] += temp.z;
    }

    gyro_bias[0] /= 100;
    gyro_bias[1] /= 100;
    gyro_bias[2] /= 100;
    UARTprintf("CHECK: Gryo bias (x, y, z) is (%x, %x, %x)\n", gyro_bias[0], gyro_bias[1], gyro_bias[2]);
    // Construct the gyro biases for push to the hardware gyro bias registers,
    // which are reset to zero upon device startup.
    // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format.
    // Biases are additive, so change sign on calculated average gyro biases
    gyro_offset[0] = (-gyro_bias[0] / 4  >> 8) & 0xFF;
    gyro_offset[1] = (-gyro_bias[0] / 4)       & 0xFF;
    gyro_offset[2] = (-gyro_bias[1] / 4  >> 8) & 0xFF;
    gyro_offset[3] = (-gyro_bias[1] / 4)       & 0xFF;
    gyro_offset[4] = (-gyro_bias[2] / 4  >> 8) & 0xFF;
    gyro_offset[5] = (-gyro_bias[2] / 4)       & 0xFF;

    write_single_icm20948_reg(2, B2_XG_OFFS_USRH, gyro_offset[0]);
    write_single_icm20948_reg(2, B2_XG_OFFS_USRL, gyro_offset[1]);

    write_single_icm20948_reg(2, B2_YG_OFFS_USRH, gyro_offset[2]);
    write_single_icm20948_reg(2, B2_YG_OFFS_USRL, gyro_offset[3]);

    write_single_icm20948_reg(2, B2_ZG_OFFS_USRH, gyro_offset[4]);
    write_single_icm20948_reg(2, B2_ZG_OFFS_USRL, gyro_offset[5]);
}

void icm20948_accel_calibration()
{
    int i;
    axises temp;
    uint8_t temp2[8] = {0};
    uint8_t temp3[8] = {0};
    uint8_t temp4[8] = {0};

    int32_t accel_bias[3] = {0};
    int32_t accel_bias_reg[3] = {0};
    uint8_t accel_offset[6] = {0};

    for(i = 0; i < 100; i++)
    {
        icm20948_accel_read(&temp);
        accel_bias[0] += temp.x;
        accel_bias[1] += temp.y;
        accel_bias[2] += temp.z;
    }

    accel_bias[0] /= 100;
    accel_bias[1] /= 100;
    accel_bias[2] /= 100;
    UARTprintf("CHECK: Accel bias (x, y, z) is (%x, %x, %x)\n", accel_bias[0], accel_bias[1], accel_bias[2]);

    uint8_t mask_bit[3] = {0, 0, 0};

//    temp2 = read_multiple_icm20948_reg(1, B1_XA_OFFS_H, 2);
    temp2[0] = read_single_icm20948_reg(1, B1_XA_OFFS_H);
    temp2[1] = read_single_icm20948_reg(1, B1_XA_OFFS_L);
    accel_bias_reg[0] = (int32_t)(temp2[0] << 8 | temp2[1]);
    mask_bit[0] = temp2[1] & 0x01;

//    temp3 = read_multiple_icm20948_reg(1, B1_YA_OFFS_H, 2);
    temp3[0] = read_single_icm20948_reg(1, B1_YA_OFFS_H);
    temp3[1] = read_single_icm20948_reg(1, B1_YA_OFFS_L);
    accel_bias_reg[1] = (int32_t)(temp3[0] << 8 | temp3[1]);
    mask_bit[1] = temp3[1] & 0x01;

//    temp4 = read_multiple_icm20948_reg(1, B1_ZA_OFFS_H, 2);
    temp4[0] = read_single_icm20948_reg(1, B1_ZA_OFFS_H);
    temp4[1] = read_single_icm20948_reg(1, B1_ZA_OFFS_L);
    accel_bias_reg[2] = (int32_t)(temp4[0] << 8 | temp4[1]);
    mask_bit[2] = temp4[1] & 0x01;

    accel_bias_reg[0] -= (accel_bias[0] / 8);
    accel_bias_reg[1] -= (accel_bias[1] / 8);
    accel_bias_reg[2] -= (accel_bias[2] / 8);

    accel_offset[0] = (accel_bias_reg[0] >> 8) & 0xFF;
    accel_offset[1] = (accel_bias_reg[0])      & 0xFE;
    accel_offset[1] = accel_offset[1] | mask_bit[0];

    accel_offset[2] = (accel_bias_reg[1] >> 8) & 0xFF;
    accel_offset[3] = (accel_bias_reg[1])      & 0xFE;
    accel_offset[3] = accel_offset[3] | mask_bit[1];

    accel_offset[4] = (accel_bias_reg[2] >> 8) & 0xFF;
    accel_offset[5] = (accel_bias_reg[2])      & 0xFE;
    accel_offset[5] = accel_offset[5] | mask_bit[2];

//    write_multiple_icm20948_reg(1, B1_XA_OFFS_H, &accel_offset[0], 2);
//    write_multiple_icm20948_reg(1, B1_YA_OFFS_H, &accel_offset[2], 2);
//    write_multiple_icm20948_reg(1, B1_ZA_OFFS_H, &accel_offset[4], 2);
    write_single_icm20948_reg(1, B1_XA_OFFS_H, accel_offset[0]);
    write_single_icm20948_reg(1, B1_XA_OFFS_L, accel_offset[1]);
    write_single_icm20948_reg(1, B1_YA_OFFS_H, accel_offset[2]);
    write_single_icm20948_reg(1, B1_YA_OFFS_L, accel_offset[3]);
    write_single_icm20948_reg(1, B1_ZA_OFFS_H, accel_offset[4]);
    write_single_icm20948_reg(1, B1_ZA_OFFS_L, accel_offset[5]);
}

void icm20948_gyro_full_scale_select(gyro_full_scale full_scale)
{
    uint8_t new_val = read_single_icm20948_reg(2, B2_GYRO_CONFIG_1);

    switch(full_scale)
    {
        case _250dps :
            new_val |= 0x00;
            gyro_scale_factor = 131.0;
            break;
        case _500dps :
            new_val |= 0x02;
            gyro_scale_factor = 65.5;
            break;
        case _1000dps :
            new_val |= 0x04;
            gyro_scale_factor = 32.8;
            break;
        case _2000dps :
            new_val |= 0x06;
            gyro_scale_factor = 16.4;
            break;
    }

    write_single_icm20948_reg(2, B2_GYRO_CONFIG_1, new_val);
}

void icm20948_accel_full_scale_select(accel_full_scale full_scale)
{
    uint8_t new_val = read_single_icm20948_reg(2, B2_ACCEL_CONFIG);

    switch(full_scale)
    {
        case _2g :
            new_val |= 0x00;
            accel_scale_factor = 16384;
            break;
        case _4g :
            new_val |= 0x02;
            accel_scale_factor = 8192;
            break;
        case _8g :
            new_val |= 0x04;
            accel_scale_factor = 4096;
            break;
        case _16g :
            new_val |= 0x06;
            accel_scale_factor = 2048;
            break;
    }

    write_single_icm20948_reg(2, B2_ACCEL_CONFIG, new_val);
}

//
/* Static Functions */
void cs_high()
{
    ROM_GPIOPinWrite(ICM20948_SPI_CS_PIN_PORT, ICM20948_SPI_CS_PIN_NUMBER, ICM20948_SPI_CS_PIN_NUMBER);
}

void cs_low()
{
    ROM_GPIOPinWrite(ICM20948_SPI_CS_PIN_PORT, ICM20948_SPI_CS_PIN_NUMBER, 0);
}

static void select_user_bank(int ub)
{
    ICM_SPI_Write(REG_BANK_SEL, ub << 4);
}

// Remove static to expand scope - otherwise unresolved symbols error
uint32_t read_single_icm20948_reg(int ub, uint32_t reg)
{
    uint32_t reg_val;
    select_user_bank(ub);

    reg_val = ICM_SPI_Read(reg);

    return reg_val;
}

void write_single_icm20948_reg(int ub, uint32_t reg, uint32_t val)
{
    select_user_bank(ub);

    ICM_SPI_Write(reg, val);

}

//static uint32_t* read_multiple_icm20948_reg(int ub, uint8_t reg, uint8_t len)
//{
//    uint8_t read_reg = READ | reg;
//    static uint8_t reg_val[6];
//    select_user_bank(ub);
//
//    cs_low();
//    HAL_SPI_Transmit(ICM20948_SPI, &read_reg, 1, 1000);
//    HAL_SPI_Receive(ICM20948_SPI, reg_val, len, 1000);
//    cs_high();
//
//    return reg_val;
//}
//
//static void write_multiple_icm20948_reg(int ub, uint8_t reg, uint8_t* val, uint8_t len)
//{
//    uint8_t write_reg = WRITE | reg;
//    select_user_bank(ub);
//
//    cs_low();
//    HAL_SPI_Transmit(ICM20948_SPI, &write_reg, 1, 1000);
//    HAL_SPI_Transmit(ICM20948_SPI, val, len, 1000);
//    cs_high();
//}

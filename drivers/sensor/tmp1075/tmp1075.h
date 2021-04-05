/*
 * Copyright 2021 Herald Project Contributors
 * Copyright (c) 2016 Intel Corporation
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_SENSOR_TMP1075_TMP1075_H_
#define ZEPHYR_DRIVERS_SENSOR_TMP1075_TMP1075_H_

#include <device.h>
#include <drivers/gpio.h>
#include <sys/util.h>

/* See https://www.ti.com/lit/ds/symlink/tmp1075.pdf */

/* NOTE do NOT set A2 to VDD when setting the address as it will exceed 0.3V MAX limit */
#define TMP1075_I2C_ADDRESS		      DT_INST_REG_ADDR(0)

/* 2 bytes, using 12 most significant bits only */
#define TMP1075_REG_TEMP_DATA         0x00
/* Scale in micro degrees Celsius */
#define TMP1075_TEMP_SCALE            62500
/* Scale range in micro degrees Celsius (-128 to 128) */
#define TMP1075_TEMP_RANGE            256000

#define TMP1075_REG_CONFIG            0x01
#define TMP1075_FIELD_CFG_MODE        BIT(8)
#define TMP1075_FIELD_CFG_ALERT       BIT(9)
#define TMP1075_FIELD_CFG_POLARITY    BIT(10)
/* Fault settings for alerts are 2 bytes */
#define TMP1075_FIELD_CFG_FAULT_START BIT(11)
/* Rate settings are 2 bytes */
#define TMP1075_FIELD_CFG_RATE_START  BIT(13)
#define TMP1075_FIELD_CFG_ONESHOT     BIT(15)

#define TMP1075_REG_LOW_LIMIT         0x02
#define TMP1075_REG_HIGH_LIMIT        0x03

#define TMP1075_REG_DEVICE_ID         0x0F
#define TMP1075_DEVICE_ID             0x7500

struct tmp1075_data {
    const struct device *i2c;
    int16_t sample;
};

/* TODO support trigger capabilities later (higher power use) */
int tmp1075_reg_read(struct tmp1075_data *drv_data, uint8_t reg, uint16_t *val);
int tmp1075_reg_write(struct tmp1075_data *drv_data, uint8_t reg, uint16_t val);
int tmp1075_reg_update(struct tmp1075_data *drv_data, uint8_t reg, uint16_t mask, uint16_t val);

/* TODO support attribute setting, triggers, interrupts */

#endif /* ZEPHYR_DRIVERS_SENSOR_TMP1075_TMP1075_H_ */
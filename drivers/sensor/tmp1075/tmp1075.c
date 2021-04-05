/*
 * Copyright 2021 Herald Project Contributors
 * Copyright (c) 2016 Intel Corporation
 * 
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT tmp1075

#include <device.h>
#include <drivers/i2c.h>
#include <drivers/gpio.h>
#include <sys/byteorder.h>
#include <sys/util.h>
#include <kernel.h>
#include <drivers/sensor.h>
#include <sys/__assert.h>
#include <logging/log.h>

#include "tmp1075.h"

LOG_MODULE_REGISTER(TMP1075, CONFIG_SENSOR_LOG_LEVEL);

int tmp1075_reg_read(struct tmp1075_data *drv_data, uint8_t reg, uint16_t *val)
{
	if (i2c_burst_read(drv_data->i2c, TMP1075_I2C_ADDRESS,
				reg, (uint8_t *) val, 2) < 0) {
		LOG_ERR("I2C read failed");
		return -EIO;
	}

	*val = sys_be16_to_cpu(*val);

	return 0;
}

int tmp1075_reg_write(struct tmp1075_data *drv_data, uint8_t reg, uint16_t val)
{
	uint8_t tx_buf[3] = {reg, val >> 8, val & 0xFF};

	return i2c_write(drv_data->i2c, tx_buf, sizeof(tx_buf),
			 TMP1075_I2C_ADDRESS);
}

int tmp1075_reg_update(struct tmp1075_data *drv_data, uint8_t reg, uint16_t mask, uint16_t val)
{
	uint16_t old_val = 0U;
	uint16_t new_val;

	if (tmp1075_reg_read(drv_data, reg, &old_val) < 0) {
		return -EIO;
	}

	new_val = old_val & ~mask;
	new_val |= val & mask;

	return tmp1075_reg_write(drv_data, reg, new_val);
}

static int tmp1075_sample_fetch(const struct device *dev,
			       enum sensor_channel chan)
{
	struct tmp1075_data *drv_data = dev->data;
	uint16_t val;

	__ASSERT_NO_MSG(chan == SENSOR_CHAN_ALL || chan == SENSOR_CHAN_AMBIENT_TEMP);

	if (tmp1075_reg_read(drv_data, TMP1075_REG_TEMP_DATA, &val) < 0) {
		return -EIO;
	}

  // No invalid data bit to check on TMP1075

	drv_data->sample = arithmetic_shift_right((int16_t)val, 2);

	return 0;
}

static int tmp1075_channel_get(const struct device *dev,
			       enum sensor_channel chan,
			       struct sensor_value *val)
{
	struct tmp1075_data *drv_data = dev->data;
	int32_t uval;

	if (chan != SENSOR_CHAN_AMBIENT_TEMP) {
		return -ENOTSUP;
	}

	uval = (int32_t)drv_data->sample * TMP1075_TEMP_SCALE;
	val->val1 = uval / 1000000;
	val->val2 = uval % 1000000;

	return 0;
}

static const struct sensor_driver_api tmp1075_driver_api = {
	.sample_fetch = tmp1075_sample_fetch,
	.channel_get = tmp1075_channel_get,
};

int tmp1075_init(const struct device *dev)
{
	struct tmp1075_data *drv_data = dev->data;

	drv_data->i2c = device_get_binding(DT_INST_BUS_LABEL(0));
	if (drv_data->i2c == NULL) {
		LOG_DBG("Failed to get pointer to %s device!",
			    DT_INST_BUS_LABEL(0));
		return -EINVAL;
	}

	// Now try and read the device ID to ensure it is correct
	uint16_t device_id = 0;
	if (tmp1075_reg_read(drv_data, TMP1075_REG_DEVICE_ID, &device_id) < 0) {
		LOG_DBG("Failed to fetch device ID");
		return -EINVAL;
	}
	if (TMP1075_DEVICE_ID != device_id) { // TODO check type and endianness
		LOG_DBG("Incorrect device ID: %d, Expected: %d", device_id, TMP1075_DEVICE_ID);
		return -EINVAL;
	}

	return 0;
}

struct tmp1075_data tmp1075_driver;

DEVICE_DT_INST_DEFINE(0, tmp1075_init, device_pm_control_nop,
		    &tmp1075_driver,
		    NULL, POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY,
		    &tmp1075_driver_api);
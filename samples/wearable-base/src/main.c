/*
 * Copyright (c) 2021 Herald Project Contributors
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>

// NOTE LIS2DW12 SA0 -> VDD then address is 0011001b (0x19)
//               SA0 -> GND then address is 0011000b (0x18)
// NOTE LIS2DE12 SAME AS ABOVE
static void fetch_and_display_accel(const struct device *sensor)
{
	static unsigned int count;
	struct sensor_value accel[3];
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (0 == rc) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}
	if (rc < 0) {
		printf("ERROR: Update failed: %d\n", rc);
	} else {
		printf("#%u @ %u ms: %sx %d.%d , y %d.%d , z %d.%d\n",
		       count, k_uptime_get_32(), overrun,
		       accel[0].val1, accel[0].val2,
		       accel[1].val1, accel[1].val2,
					 accel[2].val1, accel[2].val2);
	}
}

// NOTE TMP1075 A0->VDD, A1->GND, A2->GND then address is 0000001b (0x01)
static void fetch_and_display_temp(const struct device *sensor)
{
	static unsigned int count;
	struct sensor_value temp;
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		rc = 0;
	}
	if (0 == rc) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_AMBIENT_TEMP,
					&temp);
	}
	if (rc < 0) {
		printf("ERROR: Update failed: %d\n", rc);
	} else {
		printf("#%u @ %u ms: %stemp: %d.%d degrees Celsius\n",
		       count, k_uptime_get_32(), overrun,
		       temp.val1, temp.val2);
	}
}


void main(void)
{
	// Note: We don't return on bad initialisation in THIS sample because there may be a single
	//       device not working out of several. We have checks in the loop instead.
	const struct device *accelSensor = device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh)));
	if (NULL == accelSensor) {
		printf("Could not get %s device\n",
		       DT_LABEL(DT_INST(0, st_lis2dh)));
	}

	const struct device *tempSensor = device_get_binding(DT_LABEL(DT_INST(0, tmp1075)));
	if (NULL == tempSensor) {
		printf("Could not get %s device\n",
		       DT_LABEL(DT_INST(0, tmp1075)));
	}

	printf("Polling at 0.5 Hz\n");
	while (true) {
		if (NULL != accelSensor) {
			fetch_and_display_accel(accelSensor);
		}
		if (NULL != tempSensor) {
			fetch_and_display_temp(tempSensor);
		}
		k_sleep(K_MSEC(2000));
	}
}

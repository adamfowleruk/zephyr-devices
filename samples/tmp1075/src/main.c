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

// NOTE TMP1075 A0->VDD, A1->GND, A2->GND then address is 0000001b (0x01)

static void fetch_and_display(const struct device *sensor)
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
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_AMBIENT_TEMP,
					&temp);
	}
	if (rc < 0) {
		printf("ERROR: Update failed: %d\n", rc);
	} else {
		printf("#%u @ %u %s temp: %d.%d degrees Celsius\n",
		       count, k_uptime_get_32(), overrun,
		       temp.val1, temp.val2);
	}
}

void main(void)
{
	const struct device *sensor = device_get_binding(DT_LABEL(DT_INST(0, tmp1075)));

	if (sensor == NULL) {
		printf("Could not get %s device\n",
		       DT_LABEL(DT_INST(0, tmp1075)));
		return;
	}

	printf("Polling at 0.5 Hz\n");
	while (true) {
		fetch_and_display(sensor);
		k_sleep(K_MSEC(2000));
	}
}

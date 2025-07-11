// SPDX-License-Identifier: GPL-2.0
/*
* Copyright (C) 2025 Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <https://www.gnu.org/licenses/>
*/

#include "i2c-psoc4-driver.h"

static int irq_number;

// Interrupt handler
static irqreturn_t psoc4_irq_handler(int irq, void *dev_id)
{
	struct i2c_client *client = dev_id;
	u8 int_status;
	int ret;

	// Read the INT_STATUS register
	ret = psoc4_read_register(client, REG_INT_STATUS, &int_status, REG_INT_STATUS_SIZE);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read INT_STATUS register\n");
		psoc4_irq_clear(client);
		return IRQ_NONE;
	}

	dev_dbg(&client->dev, "INT_STATUS: 0x%02x\n", int_status);

	// Handle each interrupt type
	if (int_status & INT_STATUS_SCAN_COMPLETE) {
		dev_dbg(&client->dev, "Scan Complete interrupt\n");
		// Add specific handling for Scan Complete
	}
	if (int_status & INT_STATUS_TOUCH_DETECTED) {
		dev_dbg(&client->dev, "Touch Detected interrupt\n");
		ret = psoc4_touch_detected_handler(client);
		if (ret < 0)
			return IRQ_NONE; // No touch detected, exit early
	}
	if (int_status & INT_STATUS_TEST_RESULT_READY) {
		dev_info(&client->dev, "Test Result Ready interrupt\n");
		// Add specific handling for Test Result Ready
	}
	if (int_status & INT_STATUS_SENSING_RUNNING) {
		dev_dbg(&client->dev, "Sensing App Running interrupt\n");
		// Add specific handling for Sensing App Running
	}
	if (int_status & INT_STATUS_GEST_DETECTED) {
		dev_dbg(&client->dev, "Gesture Detected interrupt\n");
		ret = psoc4_gesture_detected_handler(client);
		if (ret < 0)
			return IRQ_NONE; // No gestures detected, exit early
	}
	if (int_status & INT_STATUS_APP_ERROR) {
		dev_err(&client->dev, "PSOC4 FW application Error interrupt\n");
		// Add specific handling for Application Error
	}

	// Clear all pending interrupts by writing 0x00 to the INT_STATUS register
	ret = psoc4_irq_clear(client);

	return IRQ_HANDLED;
}

int psoc4_irq_register(struct i2c_client *client)
{
	int ret;

	struct device_node *node = client->dev.of_node;

	// Get the interrupt number from the device tree
	irq_number = of_irq_get(node, 0);
	if (irq_number < 0) {
		dev_err(&client->dev, "Failed to get IRQ number from device tree\n");
		return irq_number;
	}

	// Request the interrupt
	ret = devm_request_threaded_irq(&client->dev, irq_number, NULL, psoc4_irq_handler,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "psoc4_irq", client);
	if (ret)
		return ret;

	dev_info(&client->dev, "Requested IRQ %d for PSOC4 FW\n", irq_number);
	return 0;
}

int psoc4_irq_clear(struct i2c_client *client)
{
	int ret;
	u8 int_status = INT_STATUS_CLEAR_PENDING;

	// Clear all pending interrupts by writing 0x00 to the INT_STATUS register
	// TODO: Review this approach, as it may not be the best way to clear interrupts
	do {
		ret = psoc4_write_register(client, REG_INT_STATUS, &int_status,
									REG_INT_STATUS_SIZE);
	} while (ret < 0);

	return 0;
}

int psoc4_touch_detected_handler(struct i2c_client *client)
{
	int ret;
	struct psoc4_touch touches[NUM_TOUCH_SLOTS];
	u8 num_touches;

	ret = psoc4_read_register(client, REG_NUM_TOUCH, &num_touches, REG_NUM_TOUCH_SIZE);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read number of touches\n");
		psoc4_irq_clear(client);
		return ret;
	}
	dev_dbg(&client->dev, "Number of touches detected: %u\n", num_touches);

	if (num_touches > NUM_TOUCH_SLOTS) { // We only support up to 2 touches
		dev_warn(&client->dev, "Unexpected number of touches: %u\n", num_touches);
		psoc4_irq_clear(client);
		return -EINVAL;
	}

	for (unsigned int slot = 0; slot < num_touches; slot++) {
		dev_dbg(&client->dev, "TCH%u detected\n", slot);

		ret = psoc4_read_xyz_coords(client, REG_TCH0_POS + (slot * REG_TCH_XYZ_SIZE_BYTES),
									&touches[slot].x,
									&touches[slot].y,
									&touches[slot].z);

		if (ret < 0) {
			dev_err(&client->dev, "Failed to read TCH%u coordinates\n", slot);
			psoc4_irq_clear(client);
			return ret;
		}

		dev_dbg(&client->dev, "TCH%u coordinates: X=%u, Y=%u, Z=%u\n",
				slot, touches[slot].x, touches[slot].y, touches[slot].z);
	}

	psoc4_input_report_coord(client, num_touches, touches); // Report touches to input subsystem
	return 0;
}

int psoc4_gesture_detected_handler(struct i2c_client *client)
{
	int ret;
	u32 gestures;

	ret = psoc4_read_gestures(client, &gestures);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read gestures\n");
		psoc4_irq_clear(client);
		return ret;
	}

	dev_info(&client->dev, "Gestures detected: 0x%08x\n", gestures);

	psoc4_input_report_gesture(client, gestures); // Report gestures to input subsystem
	return 0;
}

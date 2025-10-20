// SPDX-License-Identifier: GPL-2.0 OR MIT
/*
 * Copyright (C) 2025, Infineon Technologies AG, or an affiliate of Infineon Technologies AG.
 * All rights reserved.
 *
 * Licensed under either of
 *
 * GNU General Public License, Version 2.0 <https://www.gnu.org/licenses/gpl-2.0.html>
 * MIT license  <http://opensource.org/licenses/MIT>
 *
 * at your option.
 *
 * When Licensed under the GNU General Public License, Version 2.0 (the "License");
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
 * along with this program; if not, see <https://www.gnu.org/licenses/gpl-2.0.html>
 *
 * When licensed under the MIT license;
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the “Software”), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "i2c-psoc4-driver.h"

static struct sock *nl_socket;

// Helper to send netlink message
static void psoc4_send_nl_msg(const char *msg)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh;
	int msg_size = strlen(msg);
	int ret;

	if (!nl_socket)
		return;

	skb = nlmsg_new(msg_size, GFP_KERNEL);
	if (!skb) {
		pr_err("Netlink: Failed to allocate skb\n");
		return;
	}

	nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, msg_size, 0);
	if (!nlh) {
		pr_err("Netlink: Failed to put message header\n");
		kfree_skb(skb);
		return;
	}
	memcpy(nlmsg_data(nlh), msg, msg_size);

	ret = nlmsg_multicast(nl_socket, skb, 0, NETLINK_GROUP, GFP_KERNEL);
	if (ret < 0) {
	    if (ret == -ESRCH) {
	        pr_debug("Netlink: No listeners on multicast group %d\n", NETLINK_GROUP);
	    } else {
	        pr_err("Netlink: multicast send failed: %d\n", ret);
	    }
	}

	pr_debug("Netlink: Sent message: %s\n", msg);
}

static int psoc4_nl_init(void)
{
	struct netlink_kernel_cfg cfg = {
		.input = NULL,
	};

	nl_socket = netlink_kernel_create(&init_net, NETLINK_USER_TYPE, &cfg);
	if (!nl_socket) {
		pr_err("Netlink: Failed to create socket\n");
		return -ENOMEM;
	}

	pr_debug("Netlink: Created socket for interrupts\n");
	return 0;
}

void psoc4_nl_exit(void)
{
	if (nl_socket) {
		netlink_kernel_release(nl_socket);
		nl_socket = NULL;
	}
	pr_debug("Netlink: Released socket\n");
}

static int irq_number;

// Interrupt handler
static irqreturn_t psoc4_irq_handler(int irq, void *dev_id)
{
	struct i2c_client *client = dev_id;
	u8 int_status;
	int ret;
	char msg[NETLINK_MSG_LEN];

	// Read the INT_STATUS register
	ret = psoc4_read_register(client, REG_INT_STATUS, &int_status, REG_INT_STATUS_SIZE);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read INT_STATUS register\n");
		psoc4_irq_clear(client);
		return IRQ_NONE;
	}

	dev_dbg(&client->dev, "INT_STATUS: 0x%02x\n", int_status);

	// Handle each interrupt type and notify user space
	if (int_status & INT_STATUS_SCAN_COMPLETE) {
		dev_dbg(&client->dev, "Scan Complete interrupt\n");
		snprintf(msg, sizeof(msg), "SCAN_COMPLETE");
		psoc4_send_nl_msg(msg);
		// Add specific handling for Scan Complete
	}
	if (int_status & INT_STATUS_TOUCH_DETECTED) {
		dev_dbg(&client->dev, "Touch Detected interrupt\n");
		snprintf(msg, sizeof(msg), "TOUCH_DETECTED");
		psoc4_send_nl_msg(msg);
		ret = psoc4_touch_detected_handler(client);
		if (ret < 0)
			return IRQ_NONE; // No touch detected, exit early
	}
	if (int_status & INT_STATUS_TEST_RESULT_READY) {
		dev_info(&client->dev, "Test Result Ready interrupt\n");
		snprintf(msg, sizeof(msg), "TEST_RESULT_READY");
		psoc4_send_nl_msg(msg);
		// Add specific handling for Test Result Ready
	}
	if (int_status & INT_STATUS_SENSING_RUNNING) {
		dev_dbg(&client->dev, "Sensing App Running interrupt\n");
		snprintf(msg, sizeof(msg), "SENSING_RUNNING");
		psoc4_send_nl_msg(msg);
		// Add specific handling for Sensing App Running
	}
	if (int_status & INT_STATUS_GEST_DETECTED) {
		dev_dbg(&client->dev, "Gesture Detected interrupt\n");
		snprintf(msg, sizeof(msg), "GESTURE_DETECTED");
		psoc4_send_nl_msg(msg);
		ret = psoc4_gesture_detected_handler(client);
		if (ret < 0)
			return IRQ_NONE; // No gestures detected, exit early
	}
	if (int_status & INT_STATUS_LIFTOFF_TCHDWN) {
		dev_dbg(&client->dev, "Liftoff/Touchdown Detected interrupt\n");
		snprintf(msg, sizeof(msg), "LIFTOFF_TOUCHDOWN_DETECTED");
		psoc4_send_nl_msg(msg);
#if defined(TOUCHDOWN_LIFTOFF_ON_IRQ)
		ret = psoc4_liftoff_touchdown_handler(client);
		if (ret < 0)
			return IRQ_NONE; // No liftoff/touchdown event handled, exit early
#endif /* #if defined(TOUCHDOWN_LIFTOFF_ON_IRQ) */
	}
	if (int_status & INT_STATUS_APP_ERROR) {
		dev_err(&client->dev, "PSOC4 FW application Error interrupt\n");
		snprintf(msg, sizeof(msg), "APP_ERROR");
		psoc4_send_nl_msg(msg);
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

	// Netlink init
	ret = psoc4_nl_init();
	if (ret)
		return ret;

	// Get the interrupt number from the device tree
	irq_number = of_irq_get(node, 0);
	if (irq_number < 0) {
		dev_err(&client->dev, "Failed to get IRQ number from device tree\n");
		psoc4_nl_exit();
		return irq_number;
	}

	// Request the interrupt
	ret = devm_request_threaded_irq(&client->dev, irq_number, NULL, psoc4_irq_handler,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "psoc4_irq", client);
	if (ret) {
		psoc4_nl_exit();
		return ret;
	}

	dev_info(&client->dev, "Requested IRQ %d for PSOC4 FW\n", irq_number);
	return 0;
}
// Netlink cleanup should be called from module exit, not sysfs_remove

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

int psoc4_liftoff_touchdown_handler(struct i2c_client *client)
{
	int ret;

	u8 num_touches;

	ret = psoc4_read_register(client, REG_NUM_TOUCH, &num_touches, REG_NUM_TOUCH_SIZE);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to read number of touches\n");
		psoc4_irq_clear(client);
		return ret;
	}
	dev_dbg(&client->dev, "Number of touches detected: %u\n", num_touches);

	// Report liftoff/touchdown to input subsystem
	psoc4_input_report_liftoff_touchdown(client, num_touches);
	return 0;
}

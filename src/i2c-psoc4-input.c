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
#include "input-report-config.h"

static struct input_dev *touchpad_input_dev;

int psoc4_input_dev_create(struct i2c_client *client)
{
	int ret;
	struct device_node *of_node = client->dev.of_node;

	u16 max_x = 500, max_y = 200, max_pressure = 250; // defaults

	of_property_read_u16(of_node, "touchpad-max-x", &max_x);
	of_property_read_u16(of_node, "touchpad-max-y", &max_y);
	of_property_read_u16(of_node, "touchpad-max-pressure", &max_pressure);

	touchpad_input_dev = devm_input_allocate_device(&client->dev);
	if (!touchpad_input_dev) {
		dev_err(&client->dev, "Failed to allocate input device\n");
		return -ENOMEM;
	}
	touchpad_input_dev->name = "PSOC4 Touchpad";
	touchpad_input_dev->id.bustype = BUS_I2C;

	// Indicate that the device supports advanced coordinates for multi-touch
	// Using ABS_MT_* types
	input_set_abs_params(touchpad_input_dev, ABS_MT_POSITION_X, 0, max_x, 0, 0);
	input_set_abs_params(touchpad_input_dev, ABS_MT_POSITION_Y, 0, max_y, 0, 0);
#if defined(REPORT_PRESSURE)
	input_set_abs_params(touchpad_input_dev, ABS_MT_PRESSURE, 0, max_pressure, 0, 0);
#endif /* #if defined(REPORT_PRESSURE) */

#if defined(REPORT_LEGACY_COORDS)
	// Legacy coordinates can also be preserved separately if needed
	input_set_abs_params(touchpad_input_dev, ABS_X, 0, max_x, 0, 0);
	input_set_abs_params(touchpad_input_dev, ABS_Y, 0, max_y, 0, 0);
#if defined(REPORT_PRESSURE)
	input_set_abs_params(touchpad_input_dev, ABS_PRESSURE, 0, max_pressure, 0, 0);
#endif /* #if defined(REPORT_PRESSURE) */
#endif /* #if defined(REPORT_LEGACY_COORDS) */

	// Init slots for multi-touch
	ret = input_mt_init_slots(touchpad_input_dev, 2, INPUT_MT_POINTER);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize MT slots\n");
		return ret;
	}

	// Set up the input device properties for gestures
	__set_bit(EV_KEY, touchpad_input_dev->evbit);
	__set_bit(GEST_SINGLE_CLICK_KEY, touchpad_input_dev->keybit);
	__set_bit(GEST_DOUBLE_CLICK_KEY, touchpad_input_dev->keybit);
	__set_bit(GEST_SWIPE_UP_KEY, touchpad_input_dev->keybit);
	__set_bit(GEST_SWIPE_DOWN_KEY, touchpad_input_dev->keybit);
	__set_bit(GEST_SWIPE_RIGHT_KEY, touchpad_input_dev->keybit);
	__set_bit(GEST_SWIPE_LEFT_KEY, touchpad_input_dev->keybit);
	__set_bit(GEST_TOUCHDOWN_KEY, touchpad_input_dev->keybit);

	// Register input device
	ret = input_register_device(touchpad_input_dev);
	if (ret)
		return ret;

	dev_info(&client->dev, "Input device registered successfully\n");
	return 0;
}

void psoc4_input_dev_remove(struct i2c_client *client)
{
	if (touchpad_input_dev) {
		input_unregister_device(touchpad_input_dev);
		touchpad_input_dev = NULL;
	}
	dev_info(&client->dev, "Input device unregistered successfully\n");
}

void psoc4_input_report_coord(struct i2c_client *client, u8 num_touches,
								struct psoc4_touch *touches)
{
	if (!touchpad_input_dev) {
		dev_err(&client->dev, "Trying to report, but input device not registered\n");
		return;
	}

	if (num_touches > NUM_TOUCH_SLOTS) {
		dev_err(&client->dev, "Invalid number of touches: %d\n", num_touches);
		return;
	}

	for (unsigned int slot = 0; slot < num_touches; slot++) {
		input_mt_slot(touchpad_input_dev, slot);
		input_mt_report_slot_state(touchpad_input_dev, MT_TOOL_FINGER, true);
		input_report_abs(touchpad_input_dev, ABS_MT_POSITION_X, touches[slot].x);
		input_report_abs(touchpad_input_dev, ABS_MT_POSITION_Y, touches[slot].y);
	#if defined(REPORT_PRESSURE)
		input_report_abs(touchpad_input_dev, ABS_MT_PRESSURE, touches[slot].z);
	#endif /* #if defined(REPORT_PRESSURE) */
	}

	for (int slot = 0; slot < NUM_TOUCH_SLOTS; slot++) {
		input_mt_slot(touchpad_input_dev, slot);

		if (slot > (num_touches - 1))
			input_mt_report_slot_state(touchpad_input_dev, MT_TOOL_FINGER, false);
	}

#if defined(REPORT_LEGACY_COORDS)
	// Legacy coordinates for the first touch point
	if (num_touches >= 1) {
		input_report_abs(touchpad_input_dev, ABS_X, touches[0].x);
		input_report_abs(touchpad_input_dev, ABS_Y, touches[0].y);
	#if defined(REPORT_PRESSURE)
		input_report_abs(touchpad_input_dev, ABS_PRESSURE, touches[0].z);
	#endif /* #if defined(REPORT_PRESSURE) */
	}
#endif /* #if defined(REPORT_LEGACY_COORDS) */

	input_sync(touchpad_input_dev);
}

void psoc4_input_report_gesture(struct i2c_client *client, u32 gestures)
{
	if (!touchpad_input_dev) {
		dev_err(&client->dev, "Trying to report gesture, but input device not registered\n");
		return;
	}

	if (gestures & GEST_ONE_FINGER_SINGLE_CLICK) {
		dev_dbg(&client->dev, "One-finger single click gesture detected\n");
		report_instant_event(GEST_SINGLE_CLICK_KEY);
	}

	if (gestures & GEST_ONE_FINGER_DOUBLE_CLICK) {
		dev_dbg(&client->dev, "One-finger double click gesture detected\n");
		report_instant_event(GEST_DOUBLE_CLICK_KEY);
	}

	if (gestures & GEST_ONE_FINGER_SCROLL) {
		u8 scroll_direction = (gestures & GEST_SCROLL_DIRECTION_MASK) >> 15;

		switch (scroll_direction) {
		case GEST_DIRECTION_UP:
			dev_dbg(&client->dev, "One-finger scroll gesture detected: UP\n");
			break;
		case GEST_DIRECTION_DOWN:
			dev_dbg(&client->dev, "One-finger scroll gesture detected: DOWN\n");
			break;
		case GEST_DIRECTION_RIGHT:
			dev_dbg(&client->dev, "One-finger scroll gesture detected: RIGHT\n");
			break;
		case GEST_DIRECTION_LEFT:
			dev_dbg(&client->dev, "One-finger scroll gesture detected: LEFT\n");
			break;
		default:
			dev_warn(&client->dev, "Unknown one-finger scroll direction: 0x%02x\n",
					scroll_direction);
		}
	}

	if (gestures & GEST_ONE_FINGER_FLICK) {
		u8 flick_direction = (gestures & GEST_FLICK_DIRECTION_MASK) >> 23;

		switch (flick_direction) {
		case GEST_DIRECTION_UP:
			dev_dbg(&client->dev, "One-finger flick gesture detected: UP\n");
			report_instant_event(GEST_SWIPE_UP_KEY);
			break;
		case GEST_DIRECTION_DOWN:
			dev_dbg(&client->dev, "One-finger flick gesture detected: DOWN\n");
			report_instant_event(GEST_SWIPE_DOWN_KEY);
			break;
		case GEST_DIRECTION_RIGHT:
			dev_dbg(&client->dev, "One-finger flick gesture detected: RIGHT\n");
			report_instant_event(GEST_SWIPE_RIGHT_KEY);
			break;
		case GEST_DIRECTION_LEFT:
			dev_dbg(&client->dev, "One-finger flick gesture detected: LEFT\n");
			report_instant_event(GEST_SWIPE_LEFT_KEY);
			break;
		default:
			dev_warn(&client->dev, "Unknown one-finger flick direction: 0x%02x\n",
					flick_direction);
		}
	}

#if defined(TOUCHDOWN_LIFTOFF_ON_GESTURE)
	if (gestures & GEST_TOUCHDOWN) {
		dev_dbg(&client->dev, "Touchdown event detected\n");
		input_report_key(touchpad_input_dev, GEST_TOUCHDOWN_KEY, 1);
		input_sync(touchpad_input_dev);
	}

	if (gestures & GEST_LIFTOFF) {
		dev_dbg(&client->dev, "Liftoff event detected\n");
		input_report_key(touchpad_input_dev, GEST_TOUCHDOWN_KEY, 0);
		input_sync(touchpad_input_dev);
	}
#endif /* #if defined(TOUCHDOWN_LIFTOFF_ON_GESTURE) */
}

void psoc4_input_report_liftoff_touchdown(struct i2c_client *client, u8 num_touches)
{
	if (!touchpad_input_dev) {
		dev_err(&client->dev, "Trying to report liftoff/touchdown, but input device not registered\n");
		return;
	}

	if (num_touches != 0)
		input_report_key(touchpad_input_dev, GEST_TOUCHDOWN_KEY, 1);
	else
		input_report_key(touchpad_input_dev, GEST_TOUCHDOWN_KEY, 0);

	input_sync(touchpad_input_dev);
}

void report_instant_event(u32 key_code)
{
	input_report_key(touchpad_input_dev, key_code, 1);
	input_sync(touchpad_input_dev);
	input_report_key(touchpad_input_dev, key_code, 0);
	input_sync(touchpad_input_dev);
}

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

static struct dentry *psoc4_debugfs_root;

// debugfs attribute for touch0_pos (Read-Only)
static int touch0_pos_seq_show(struct seq_file *s, void *v)
{
	struct i2c_client *client = to_i2c_client(s->private);
	u16 x, y, z;
	int ret;

	ret = psoc4_read_xyz_coords(client, REG_TCH0_POS, &x, &y, &z);

	if (ret < 0)
		return ret;

	seq_printf(s, "%u %u %u\n", x, y, z);
	return 0;
}

// debugfs attribute for touch1_pos (Read-Only)
static int touch1_pos_seq_show(struct seq_file *s, void *v)
{
	struct i2c_client *client = to_i2c_client(s->private);
	u16 x, y, z;
	int ret;

	ret = psoc4_read_xyz_coords(client, REG_TCH1_POS, &x, &y, &z);
	if (ret < 0)
		return ret;

	seq_printf(s, "%u %u %u\n", x, y, z);
	return 0;
}

// debugfs attribute for num_touch (Read-Only)
static int num_touch_seq_show(struct seq_file *s, void *v)
{
	struct i2c_client *client = to_i2c_client(s->private);
	u8 value;
	int ret;

	ret = psoc4_read_register(client, REG_NUM_TOUCH, &value, REG_NUM_TOUCH_SIZE);
	if (ret < 0)
		return ret;

	seq_printf(s, "%u\n", value);
	return 0;
}

// debugfs attribute for sns_raw (Read-Only)
static int sns_raw_seq_show(struct seq_file *s, void *v)
{
	struct i2c_client *client = to_i2c_client(s->private);
	u8 num_sensors;
	u16 *raw_data;
	int ret, i;

	ret = psoc4_read_register(client, REG_NUM_SNS, &num_sensors, REG_NUM_SNS_SIZE);
	if (ret < 0)
		return ret;

	raw_data = kzalloc(REG_SNS_RAW_SIZE(num_sensors), GFP_KERNEL);
	if (!raw_data)
		return -ENOMEM;

	ret = psoc4_read_register(client, REG_SNS_RAW, (u8 *)raw_data,
					REG_SNS_RAW_SIZE(num_sensors));
	if (ret < 0) {
		kfree(raw_data);
		return ret;
	}

	for (i = 0; i < num_sensors; i++)
		seq_printf(s, "0x%04x ", le16_to_cpu(raw_data[i]));
	seq_putc(s, '\n');

	kfree(raw_data);
	return 0;
}

// debugfs attribute for sns_bsln (Read-Only)
static int sns_bsln_seq_show(struct seq_file *s, void *v)
{
	struct i2c_client *client = to_i2c_client(s->private);
	u8 num_sensors;
	u16 *bsln_data;
	int ret, i;

	ret = psoc4_read_register(client, REG_NUM_SNS, &num_sensors, REG_NUM_SNS_SIZE);
	if (ret < 0)
		return ret;

	bsln_data = kzalloc(REG_SNS_BSLN_SIZE(num_sensors), GFP_KERNEL);
	if (!bsln_data)
		return -ENOMEM;

	ret = psoc4_read_register(client, REG_SNS_BSLN(num_sensors),
				(u8 *)bsln_data, REG_SNS_BSLN_SIZE(num_sensors));
	if (ret < 0) {
		kfree(bsln_data);
		return ret;
	}

	for (i = 0; i < num_sensors; i++)
		seq_printf(s, "0x%04x ", le16_to_cpu(bsln_data[i]));
	seq_putc(s, '\n');

	kfree(bsln_data);
	return 0;
}

// debugfs attribute for sns_cp_measure
static int sns_cp_measure_seq_show(struct seq_file *s, void *v)
{
	struct i2c_client *client = to_i2c_client(s->private);
	u8 num_sensors;
	u32 *cp_data;
	int ret, i;

	ret = psoc4_read_register(client, REG_NUM_SNS, &num_sensors, REG_NUM_SNS_SIZE);
	if (ret < 0)
		return ret;

	cp_data = kzalloc(REG_SNS_CP_MEASURE_SIZE(num_sensors), GFP_KERNEL);
	if (!cp_data)
		return -ENOMEM;

	ret = psoc4_read_register(client, REG_SNS_CP_MEASURE(num_sensors),
				(u8 *)cp_data, REG_SNS_CP_MEASURE_SIZE(num_sensors));
	if (ret < 0) {
		kfree(cp_data);
		return ret;
	}

	for (i = 0; i < num_sensors; i++)
		seq_printf(s, "0x%08x ", le32_to_cpu(cp_data[i]));
	seq_putc(s, '\n');

	kfree(cp_data);
	return 0;
}

// debugfs attribute for gestures
static int gestures_raw_seq_show(struct seq_file *s, void *v)
{
	struct i2c_client *client = to_i2c_client(s->private);
	u32 gestures_raw;
	int ret;

	ret = psoc4_read_gestures(client, &gestures_raw);
	if (ret < 0)
		return ret;

	seq_printf(s, "0x%08x\n", gestures_raw);
	return 0;
}

// debugfs attribute for numSns (Read-Only)
static int num_sns_seq_show(struct seq_file *s, void *v)
{
	struct i2c_client *client = to_i2c_client(s->private);
	u8 num_sensors;
	int ret;

	ret = psoc4_read_register(client, REG_NUM_SNS, &num_sensors, REG_NUM_SNS_SIZE);
	if (ret < 0)
		return ret;

	seq_printf(s, "%u\n", num_sensors);
	return 0;
}

int psoc4_debugfs_create(struct i2c_client *client)
{
	if (!psoc4_debugfs_root) {
		psoc4_debugfs_root = debugfs_create_dir("psoc4_capsense", NULL);
		if (!psoc4_debugfs_root)
			return -ENOMEM;
	}

	debugfs_create_devm_seqfile(&client->dev, "touch0_pos",
				psoc4_debugfs_root, touch0_pos_seq_show);
	debugfs_create_devm_seqfile(&client->dev, "touch1_pos",
				psoc4_debugfs_root, touch1_pos_seq_show);
	debugfs_create_devm_seqfile(&client->dev, "num_touch",
				psoc4_debugfs_root, num_touch_seq_show);
	debugfs_create_devm_seqfile(&client->dev, "sns_raw",
				psoc4_debugfs_root, sns_raw_seq_show);
	debugfs_create_devm_seqfile(&client->dev, "sns_bsln",
				psoc4_debugfs_root, sns_bsln_seq_show);
	debugfs_create_devm_seqfile(&client->dev, "sns_cp_measure",
				psoc4_debugfs_root, sns_cp_measure_seq_show);
	debugfs_create_devm_seqfile(&client->dev, "gestures_raw",
				psoc4_debugfs_root, gestures_raw_seq_show);
	debugfs_create_devm_seqfile(&client->dev, "num_sns",
				psoc4_debugfs_root, num_sns_seq_show);

	return 0;
}

void psoc4_debugfs_remove(void)
{
	debugfs_remove(psoc4_debugfs_root);
	psoc4_debugfs_root = NULL;
}

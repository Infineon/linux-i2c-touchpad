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

// Sysfs attribute for FW_VER (read operation)
static ssize_t fw_ver_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value[REG_FW_VER_SIZE];

	int ret = psoc4_read_register(client, REG_FW_VER, value, REG_FW_VER_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "%u.%u.%u\n", value[0], value[1] | (value[2] << 8), value[3] | (value[4] << 8));
}
static DEVICE_ATTR_RO(fw_ver);

// Sysfs attribute for RST_CAUSE (read operation)
static ssize_t rst_cause_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_RST_CAUSE, &value, REG_RST_CAUSE_SIZE);

	if (value < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}
static DEVICE_ATTR_RO(rst_cause);

// Generic function to handle CMD register writes
static ssize_t cmd_bit_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count, u8 mask)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value = 0;
	int ret;

	ret = kstrtou8(buf, 16, &value);
	if (ret < 0 || value != 1) // Only accept "1" as valid input
		return -EINVAL;

	ret = psoc4_write_register(client, REG_CMD, &mask, 1);
	if (ret < 0)
		return -EIO;

	return count;
}

// Sysfs attribute for triggering a software reset
static ssize_t reset_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return cmd_bit_store(dev, attr, buf, count, CMD_BIT_RESET);
}
static DEVICE_ATTR_WO(reset);

// Sysfs attribute for triggering storage of CAPSENSE configuration
static ssize_t save_capsense_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return cmd_bit_store(dev, attr, buf, count, CMD_BIT_SAVE_CAPSENSE);
}
static DEVICE_ATTR_WO(save_capsense);

// Sysfs attribute for triggering restoration of CAPSENSE configuration
static ssize_t restore_capsense_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return cmd_bit_store(dev, attr, buf, count, CMD_BIT_RESTORE_CAPSENSE);
}
static DEVICE_ATTR_WO(restore_capsense);

// Sysfs attribute for triggering Cp Test
static ssize_t cp_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return cmd_bit_store(dev, attr, buf, count, CMD_BIT_CP_TEST);
}
static DEVICE_ATTR_WO(cp_test);

// Sysfs attribute for triggering Short Test
static ssize_t short_test_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return cmd_bit_store(dev, attr, buf, count, CMD_BIT_SHORT_TEST);
}
static DEVICE_ATTR_WO(short_test);

// Sysfs attribute for triggering Bootloader Jump
static ssize_t bootloader_jump_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	return cmd_bit_store(dev, attr, buf, count, CMD_BIT_BOOTLOADER_JUMP);
}
static DEVICE_ATTR_WO(bootloader_jump);

// Sysfs attribute for TEST_STATUS (read operation)
static ssize_t test_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 test_status;
	u32 shorted_sns_id;
	int ret;

	// Read TEST_STATUS
	ret = psoc4_read_register(client, REG_TEST_STATUS, &test_status, REG_TEST_STATUS_SIZE);
	if (ret < 0)
		return -EIO;

	// If TEST_STATUS is 0x0F, read SHORTED_SNS_ID
	if (test_status == 0x0F) {
		ret = psoc4_read_register(client, REG_SHORTED_SNS_ID, (u8 *)&shorted_sns_id, REG_SHORTED_SNS_ID_SIZE);
		if (ret < 0)
			return -EIO;

		shorted_sns_id = le32_to_cpu(shorted_sns_id);
		return sprintf(buf, "0x%02x 0x%08x\n", test_status, shorted_sns_id);
	}

	// Otherwise, return only TEST_STATUS
	return sprintf(buf, "0x%02x\n", test_status);
}
static DEVICE_ATTR_RO(test_status);

// Sysfs attribute for INT_SRC_EN (read operation)
static ssize_t int_src_en_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_INT_SRC_EN, &value, REG_INT_SRC_EN_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}

// Sysfs attribute for INT_SRC_EN (write operation)
static ssize_t int_src_en_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = kstrtou8(buf, 16, &value);

	if (ret < 0)
		return -EINVAL;

	ret = psoc4_write_register(client, REG_INT_SRC_EN, &value, REG_INT_SRC_EN_SIZE);
	if (ret < 0)
		return -EIO;

	return count;
}
static DEVICE_ATTR_RW(int_src_en);

// Sysfs attribute for INT_STATUS (read operation)
static ssize_t int_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_INT_STATUS, &value, REG_INT_STATUS_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}

// Sysfs attribute for INT_STATUS (write operation)
static ssize_t int_status_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = kstrtou8(buf, 16, &value);

	if (ret < 0)
		return -EINVAL;

	ret = psoc4_write_register(client, REG_INT_STATUS, &value, REG_INT_STATUS_SIZE);
	if (ret < 0)
		return -EIO;

	return count;
}
static DEVICE_ATTR_RW(int_status);

// Sysfs attribute for ERROR_STATUS (read operation)
static ssize_t error_status_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_ERROR_STATUS, &value, REG_ERROR_STATUS_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}
static DEVICE_ATTR_RO(error_status);

// Sysfs attribute for SCAN_MODE (read operation)
static ssize_t scan_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_SCAN_MODE, &value, REG_SCAN_MODE_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}
static DEVICE_ATTR_RO(scan_mode);

// Sysfs attribute for SHIELD_EN (read operation)
static ssize_t shield_en_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_SHIELD_EN, &value, REG_SHIELD_EN_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}

// Sysfs attribute for SHIELD_EN (write operation)
static ssize_t shield_en_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = kstrtou8(buf, 16, &value);

	if (ret < 0)
		return -EINVAL;

	ret = psoc4_write_register(client, REG_SHIELD_EN, &value, REG_SHIELD_EN_SIZE);
	if (ret < 0)
		return -EIO;

	return count;
}
static DEVICE_ATTR_RW(shield_en);

// Sysfs attribute for WEAR_DET_EN (read operation)
static ssize_t wear_det_en_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_WEAR_DET_EN, &value, REG_WEAR_DET_EN_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}

// Sysfs attribute for WEAR_DET_EN (write operation)
static ssize_t wear_det_en_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = kstrtou8(buf, 16, &value);

	if (ret < 0)
		return -EINVAL;

	ret = psoc4_write_register(client, REG_WEAR_DET_EN, &value, REG_WEAR_DET_EN_SIZE);
	if (ret < 0)
		return -EIO;

	return count;
}
static DEVICE_ATTR_RW(wear_det_en);

// Sysfs attribute for SNS_AUTO_CAL_EN (read operation)
static ssize_t sns_auto_cal_en_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_SNS_AUTO_CAL_EN, &value, REG_SNS_AUTO_CAL_EN_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}

// Sysfs attribute for SNS_AUTO_CAL_EN (write operation)
static ssize_t sns_auto_cal_en_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = kstrtou8(buf, 16, &value);

	if (ret < 0)
		return -EINVAL;

	ret = psoc4_write_register(client, REG_SNS_AUTO_CAL_EN, &value, REG_SNS_AUTO_CAL_EN_SIZE);
	if (ret < 0)
		return -EIO;

	return count;
}
static DEVICE_ATTR_RW(sns_auto_cal_en);

// Sysfs attribute for SNS_FILT_CFG (read operation)
static ssize_t sns_filt_cfg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u16 value;
	int ret;

	ret = psoc4_read_register(client, REG_SNS_FILT_CFG, (u8 *)&value, REG_SNS_FILT_CFG_SIZE);

	if (ret < 0)
		return -EIO;

	value = le16_to_cpu(value);
	return sprintf(buf, "0x%04x\n", value);
}

// Sysfs attribute for SNS_FILT_CFG (write operation)
static ssize_t sns_filt_cfg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	u16 value;
	u8 data[2];

	int ret = kstrtou16(buf, 16, &value);

	if (ret < 0)
		return -EINVAL;

	data[0] = value & 0xFF;
	data[1] = (value >> 8) & 0xFF;

	ret = psoc4_write_register(client, REG_SNS_FILT_CFG, data, REG_SNS_FILT_CFG_SIZE);
	if (ret < 0)
		return -EIO;

	return count;
}
static DEVICE_ATTR_RW(sns_filt_cfg);

// Sysfs attribute for SNS_REF_RATE_ACT (read operation)
static ssize_t sns_ref_rate_act_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_SNS_REF_RATE_ACT, &value, REG_SNS_REF_RATE_ACT_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}

// Sysfs attribute for SNS_REF_RATE_ACT (write operation)
static ssize_t sns_ref_rate_act_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = kstrtou8(buf, 16, &value);

	if (ret < 0)
		return -EINVAL;

	ret = psoc4_write_register(client, REG_SNS_REF_RATE_ACT, &value, REG_SNS_REF_RATE_ACT_SIZE);
	if (ret < 0)
		return -EIO;

	return count;
}
static DEVICE_ATTR_RW(sns_ref_rate_act);

// Sysfs attribute for SNS_REF_RATE_ALR (read operation)
static ssize_t sns_ref_rate_alr_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = psoc4_read_register(client, REG_SNS_REF_RATE_ALR, &value, REG_SNS_REF_RATE_ALR_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "0x%02x\n", value);
}

// Sysfs attribute for SNS_REF_RATE_ALR (write operation)
static ssize_t sns_ref_rate_alr_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;

	int ret = kstrtou8(buf, 16, &value);

	if (ret < 0)
		return -EINVAL;

	ret = psoc4_write_register(client, REG_SNS_REF_RATE_ALR, &value, REG_SNS_REF_RATE_ALR_SIZE);
	if (ret < 0)
		return -EIO;

	return count;
}
static DEVICE_ATTR_RW(sns_ref_rate_alr);

// Sysfs attribute for TOUCH0_POS (read operation)
// TODO: Move this attribute to debugfs after touchscreen representation is implemented
static ssize_t touch0_pos_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u16 x, y, z;
	int ret;

	ret = psoc4_read_xyz_coords(client, REG_TCH0_POS, &x, &y, &z);
	if (ret < 0)
		return -EIO;

	return sprintf(buf, "%u %u %u\n", x, y, z);
}
static DEVICE_ATTR_RO(touch0_pos);

// Sysfs attribute for TOUCH1_POS (read operation)
// TODO: Move this attribute to debugfs after touchscreen representation is implemented
static ssize_t touch1_pos_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u16 x, y, z;
	int ret;

	ret = psoc4_read_xyz_coords(client, REG_TCH1_POS, &x, &y, &z);
	if (ret < 0)
		return -EIO;

	return sprintf(buf, "%u %u %u\n", x, y, z);
}
static DEVICE_ATTR_RO(touch1_pos);

// Sysfs attribute for NUM_TOUCH (read operation)
static ssize_t num_touch_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 value;
	int ret;

	ret = psoc4_read_register(client, REG_NUM_TOUCH, &value, REG_NUM_TOUCH_SIZE);

	if (ret < 0)
		return -EIO;

	return sprintf(buf, "%u\n", value);
}
static DEVICE_ATTR_RO(num_touch);

// Sysfs attribute for SNS_RAW (read operation)
static ssize_t sns_raw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 num_sensors;
	u16 *raw_data;
	int ret, i, offset = 0;

	// Read the number of sensors
	ret = psoc4_read_register(client, REG_NUM_SNS, &num_sensors, REG_NUM_SNS_SIZE);
	if (ret < 0)
		return -EIO;

	raw_data = kzalloc(num_sensors * 2, GFP_KERNEL);
	if (!raw_data)
		return -ENOMEM;

	// Read SNS_RAW data
	ret = psoc4_read_register(client, REG_SNS_RAW, (u8 *)raw_data,
								num_sensors * sizeof(*raw_data));
	if (ret < 0) {
		kfree(raw_data);
		return -EIO;
	}

	// Format the output
	for (i = 0; i < num_sensors; i++)
		offset += sprintf(buf + offset, "0x%04x ", le16_to_cpu(raw_data[i]));

	kfree(raw_data);
	return offset;
}
static DEVICE_ATTR_RO(sns_raw);

// Sysfs attribute for SNS_BSLN (read operation)
static ssize_t sns_bsln_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 num_sensors;
	u16 *bsln_data;
	int ret, i, offset = 0;

	// Read the number of sensors
	ret = psoc4_read_register(client, REG_NUM_SNS, &num_sensors, REG_NUM_SNS_SIZE);
	if (ret < 0)
		return -EIO;

	bsln_data = kzalloc(num_sensors * 2, GFP_KERNEL);
	if (!bsln_data)
		return -ENOMEM;

	// Read SNS_BSLN data
	ret = psoc4_read_register(client, REG_SNS_BSLN(num_sensors), (u8 *)bsln_data,
								num_sensors * sizeof(*bsln_data));
	if (ret < 0) {
		kfree(bsln_data);
		return -EIO;
	}

	// Format the output
	for (i = 0; i < num_sensors; i++)
		offset += sprintf(buf + offset, "0x%04x ", le16_to_cpu(bsln_data[i]));

	kfree(bsln_data);
	return offset;
}
static DEVICE_ATTR_RO(sns_bsln);

// Sysfs attribute for SNS_CP_MEASURE (read operation)
static ssize_t sns_cp_measure_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	u8 num_sensors;
	u32 *cp_data;
	int ret, i, offset = 0;

	// Read the number of sensors
	ret = psoc4_read_register(client, REG_NUM_SNS, &num_sensors, REG_NUM_SNS_SIZE);
	if (ret < 0)
		return -EIO;

	cp_data = kzalloc(num_sensors * 4, GFP_KERNEL);
	if (!cp_data)
		return -ENOMEM;

	// Read SNS_CP_MEASURE data
	ret = psoc4_read_register(client, REG_SNS_CP_MEASURE(num_sensors), (u8 *)cp_data,
								num_sensors * sizeof(*cp_data));
	if (ret < 0) {
		kfree(cp_data);
		return -EIO;
	}

	// Format the output
	for (i = 0; i < num_sensors; i++)
		offset += sprintf(buf + offset, "0x%08x ", le32_to_cpu(cp_data[i]));

	kfree(cp_data);
	return offset;
}
static DEVICE_ATTR_RO(sns_cp_measure);

// Function to create sysfs entries
int psoc4_sysfs_create(struct i2c_client *client)
{
	int ret;

	/* Create a symbolic link directly under /sys/bus/i2c/devices/ */
	ret = sysfs_create_link(&client->dev.parent->kobj,
							&client->dev.kobj,
							"psoc4-capsense");
	if (ret) {
		dev_err(&client->dev, "Failed to create sysfs symlink\n");
		return ret;
	}

	ret = device_create_file(&client->dev, &dev_attr_fw_ver);
	if (ret)
		goto remove_sysfs_link;

	ret = device_create_file(&client->dev, &dev_attr_rst_cause);
	if (ret)
		goto remove_fw_ver;

	ret = device_create_file(&client->dev, &dev_attr_reset);
	if (ret)
		goto remove_rst_cause;

	ret = device_create_file(&client->dev, &dev_attr_save_capsense);
	if (ret)
		goto remove_reset;

	ret = device_create_file(&client->dev, &dev_attr_restore_capsense);
	if (ret)
		goto remove_save_capsense;

	ret = device_create_file(&client->dev, &dev_attr_cp_test);
	if (ret)
		goto remove_restore_capsense;

	ret = device_create_file(&client->dev, &dev_attr_short_test);
	if (ret)
		goto remove_cp_test;

	ret = device_create_file(&client->dev, &dev_attr_bootloader_jump);
	if (ret)
		goto remove_short_test;

	ret = device_create_file(&client->dev, &dev_attr_test_status);
	if (ret)
		goto remove_bootloader_jump;

	ret = device_create_file(&client->dev, &dev_attr_int_src_en);
	if (ret)
		goto remove_test_status;

	ret = device_create_file(&client->dev, &dev_attr_int_status);
	if (ret)
		goto remove_int_src_en;

	ret = device_create_file(&client->dev, &dev_attr_error_status);
	if (ret)
		goto remove_int_status;

	ret = device_create_file(&client->dev, &dev_attr_scan_mode);
	if (ret)
		goto remove_error_status;

	ret = device_create_file(&client->dev, &dev_attr_shield_en);
	if (ret)
		goto remove_scan_mode;

	ret = device_create_file(&client->dev, &dev_attr_wear_det_en);
	if (ret)
		goto remove_shield_en;

	ret = device_create_file(&client->dev, &dev_attr_sns_auto_cal_en);
	if (ret)
		goto remove_wear_det_en;

	ret = device_create_file(&client->dev, &dev_attr_sns_filt_cfg);
	if (ret)
		goto remove_sns_auto_cal_en;

	ret = device_create_file(&client->dev, &dev_attr_sns_ref_rate_act);
	if (ret)
		goto remove_sns_filt_cfg;

	ret = device_create_file(&client->dev, &dev_attr_sns_ref_rate_alr);
	if (ret)
		goto remove_sns_ref_rate_act;

	ret = device_create_file(&client->dev, &dev_attr_touch0_pos);
	if (ret)
		goto remove_sns_ref_rate_alr;

	ret = device_create_file(&client->dev, &dev_attr_touch1_pos);
	if (ret)
		goto remove_touch0_pos;

	ret = device_create_file(&client->dev, &dev_attr_num_touch);
	if (ret)
		goto remove_touch1_pos;

	ret = device_create_file(&client->dev, &dev_attr_sns_raw);
	if (ret)
		goto remove_num_touch;

	ret = device_create_file(&client->dev, &dev_attr_sns_bsln);
	if (ret)
		goto remove_sns_raw;

	ret = device_create_file(&client->dev, &dev_attr_sns_cp_measure);
	if (ret)
		goto remove_sns_bsln;

	return 0;

remove_sns_bsln:
	device_remove_file(&client->dev, &dev_attr_sns_bsln);
remove_sns_raw:
	device_remove_file(&client->dev, &dev_attr_sns_raw);
remove_num_touch:
	device_remove_file(&client->dev, &dev_attr_num_touch);
remove_touch1_pos:
	device_remove_file(&client->dev, &dev_attr_touch1_pos);
remove_touch0_pos:
	device_remove_file(&client->dev, &dev_attr_touch0_pos);
remove_sns_ref_rate_alr:
	device_remove_file(&client->dev, &dev_attr_sns_ref_rate_alr);
remove_sns_ref_rate_act:
	device_remove_file(&client->dev, &dev_attr_sns_ref_rate_act);
remove_sns_filt_cfg:
	device_remove_file(&client->dev, &dev_attr_sns_filt_cfg);
remove_sns_auto_cal_en:
	device_remove_file(&client->dev, &dev_attr_sns_auto_cal_en);
remove_wear_det_en:
	device_remove_file(&client->dev, &dev_attr_wear_det_en);
remove_shield_en:
	device_remove_file(&client->dev, &dev_attr_shield_en);
remove_scan_mode:
	device_remove_file(&client->dev, &dev_attr_scan_mode);
remove_error_status:
	device_remove_file(&client->dev, &dev_attr_error_status);
remove_int_status:
	device_remove_file(&client->dev, &dev_attr_int_status);
remove_int_src_en:
	device_remove_file(&client->dev, &dev_attr_int_src_en);
remove_test_status:
	device_remove_file(&client->dev, &dev_attr_test_status);
remove_bootloader_jump:
	device_remove_file(&client->dev, &dev_attr_bootloader_jump);
remove_short_test:
	device_remove_file(&client->dev, &dev_attr_short_test);
remove_cp_test:
	device_remove_file(&client->dev, &dev_attr_cp_test);
remove_restore_capsense:
	device_remove_file(&client->dev, &dev_attr_restore_capsense);
remove_save_capsense:
	device_remove_file(&client->dev, &dev_attr_save_capsense);
remove_reset:
	device_remove_file(&client->dev, &dev_attr_reset);
remove_rst_cause:
	device_remove_file(&client->dev, &dev_attr_rst_cause);
remove_fw_ver:
	device_remove_file(&client->dev, &dev_attr_fw_ver);
remove_sysfs_link:
	sysfs_remove_link(&client->dev.parent->kobj, "psoc4-capsense");
	return ret;
}

// Function to remove sysfs entries
void psoc4_sysfs_remove(struct i2c_client *client)
{
	device_remove_file(&client->dev, &dev_attr_fw_ver);
	device_remove_file(&client->dev, &dev_attr_rst_cause);
	device_remove_file(&client->dev, &dev_attr_reset);
	device_remove_file(&client->dev, &dev_attr_save_capsense);
	device_remove_file(&client->dev, &dev_attr_restore_capsense);
	device_remove_file(&client->dev, &dev_attr_cp_test);
	device_remove_file(&client->dev, &dev_attr_short_test);
	device_remove_file(&client->dev, &dev_attr_bootloader_jump);
	device_remove_file(&client->dev, &dev_attr_test_status);
	device_remove_file(&client->dev, &dev_attr_int_src_en);
	device_remove_file(&client->dev, &dev_attr_int_status);
	device_remove_file(&client->dev, &dev_attr_error_status);
	device_remove_file(&client->dev, &dev_attr_scan_mode);
	device_remove_file(&client->dev, &dev_attr_shield_en);
	device_remove_file(&client->dev, &dev_attr_wear_det_en);
	device_remove_file(&client->dev, &dev_attr_sns_auto_cal_en);
	device_remove_file(&client->dev, &dev_attr_sns_filt_cfg);
	device_remove_file(&client->dev, &dev_attr_sns_ref_rate_act);
	device_remove_file(&client->dev, &dev_attr_sns_ref_rate_alr);
	device_remove_file(&client->dev, &dev_attr_touch0_pos);
	device_remove_file(&client->dev, &dev_attr_touch1_pos);
	device_remove_file(&client->dev, &dev_attr_num_touch);
	device_remove_file(&client->dev, &dev_attr_sns_raw);
	device_remove_file(&client->dev, &dev_attr_sns_bsln);
	device_remove_file(&client->dev, &dev_attr_sns_cp_measure);

	sysfs_remove_link(&client->dev.parent->kobj, "psoc4-capsense");
}

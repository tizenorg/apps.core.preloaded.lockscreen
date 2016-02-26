/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "log.h"
#include "data_model_battery.h"

#include <device/battery.h>
#include <device/callback.h>
#include <runtime_info.h>

#include <Ecore.h>

static lockscreen_data_model_t *current;

int _battery_status_update()
{
	bool model_changed = false;

	int percent, ret;
	bool is_charging, is_connected;

	ret = runtime_info_get_value_bool(RUNTIME_INFO_KEY_CHARGER_CONNECTED, &is_connected);
	if (ret != RUNTIME_INFO_ERROR_NONE) {
		_E("runtime_info_get_value_bool failed: %s", get_error_message(ret));
		return -1;
	}
	if (current->battery.is_connected != is_connected) {
		current->battery.is_connected = is_connected;
		model_changed = true;
	}

	ret = device_battery_is_charging(&is_charging);
	if (ret != DEVICE_ERROR_NONE) {
		_E("device_battery_is_charging failed: %s", get_error_message(ret));
		return -1;
	}
	if (current->battery.is_charging != is_charging) {
		current->battery.is_charging = is_charging;
		model_changed = true;
	}

	ret = device_battery_get_percent(&percent);
	if (ret != DEVICE_ERROR_NONE) {
		_E("device_battery_get_percent failed: %s", get_error_message(ret));
		return -1;
	}
	if (current->battery.level != percent) {
		current->battery.level = percent;
		model_changed = true;
	}

	if (model_changed)
		lockscreen_data_model_event_emit(LOCKSCREEN_DATA_MODEL_EVENT_BATTERY_CHANGED);

	return 0;
}

static void _battery_changed_cb(device_callback_e type, void *value, void *user_data)
{
	if (type == DEVICE_CALLBACK_BATTERY_LEVEL || type == DEVICE_CALLBACK_BATTERY_CAPACITY)
		_battery_status_update();
}

static void _battery_charger_changed_cb(runtime_info_key_e key, void *data)
{
	if (key == RUNTIME_INFO_KEY_CHARGER_CONNECTED)
		_battery_status_update();
}

int lockscreen_data_model_battery_init(lockscreen_data_model_t *model)
{
	if (current)
		return -1;

	int ret = runtime_info_set_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED, _battery_charger_changed_cb, NULL);
	if (ret != RUNTIME_INFO_ERROR_NONE) {
		_E("runtime_info_set_changed_cb failed: %s", get_error_message(ret));
		return -1;
	}

	ret = device_add_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, _battery_changed_cb, NULL);
	if (ret != DEVICE_ERROR_NONE) {
		_E("device_add_callback failed: %s", get_error_message(ret));
		runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED);
		return -1;
	}

	ret = device_add_callback(DEVICE_CALLBACK_BATTERY_CHARGING, _battery_changed_cb, NULL);
	if (ret != DEVICE_ERROR_NONE) {
		_E("device_add_callback failed: %s", get_error_message(ret));
		device_remove_callback(DEVICE_CALLBACK_BATTERY_CAPACITY, _battery_changed_cb);
		runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED);
		return -1;
	}

	current = model;
	_battery_status_update();

	return 0;
}

void lockscreen_data_model_battery_shutdown(void)
{
	if (!current)
		return;

	runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_CHARGER_CONNECTED);

	current = NULL;
}

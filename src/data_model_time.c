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

#include "data_model.h"
#include "log.h"
#include <system_settings.h>

static lockscreen_data_model_t *current;

static void _time_changed(system_settings_key_e key, void *user_data)
{
	int ret = SYSTEM_SETTINGS_ERROR_NOT_SUPPORTED;

	switch (key) {
		case SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR:
			ret = system_settings_set_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, &current->time.use24hformat);
			break;
		case SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE:
			free(current->time.locale);
			ret = system_settings_set_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, &current->time.locale);
			break;
		case SYSTEM_SETTINGS_KEY_TIME_CHANGED:
			break;
		default:
			_E("Unhandled system_setting event: %d", key);
			break;
	}

	if (ret == SYSTEM_SETTINGS_ERROR_NONE) {
		lockscreen_data_model_event_emit(LOCKSCREEN_DATA_MODEL_EVENT_TIME_FORMAT_CHANGED);
	}
}

int lockscreen_data_model_time_init(lockscreen_data_model_t *model)
{
	int ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, _time_changed, NULL);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		_E("system_settings_set_changed_cb failed: %s", get_error_message(ret));
		return 1;
	}
	ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, _time_changed, NULL);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		_E("system_settings_set_changed_cb failed: %s", get_error_message(ret));
		return 1;
	}
	ret = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_TIME_CHANGED, _time_changed, NULL);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		_E("system_settings_set_changed_cb failed: %s", get_error_message(ret));
		return 1;
	}
	ret = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR, &model->time.use24hformat);

	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		_E("system_settings_get_value_bool failed: %s", get_error_message(ret));
		return 1;
	}

	ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE, &model->time.locale);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		_E("system_settings_get_value_string failed: %s", get_error_message(ret));
		return 1;
	}

	current = model;

	return 0;
}

void lockscreen_data_model_time_shutdown(void)
{
	free(current->time.locale);
	system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEFORMAT_24HOUR);
	system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_LOCALE_TIMEZONE);
	system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_TIME_CHANGED);
}

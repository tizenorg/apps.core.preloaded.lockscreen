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
#include "data_model.h"
#include "util.h"

#include <system_settings.h>
#include <Ecore_File.h>

#define DEFAULT_BG "images/Default.jpg"

static lockscreen_data_model_t *current;

int lockscreen_data_model_background_init(lockscreen_data_model_t *model)
{
	char *bg;

	if (current)
		return -1;

	int ret = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, &bg);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		_E("system_settings_set_value_string failed: %s", get_error_message(ret));
		return -1;
	}

	current = model;
	lockscreen_data_model_background_file_set(bg);

	return 0;
}

int lockscreen_data_model_background_file_set(const char *path)
{
	if (!current)
		return -1;

	if (!path) {
		return lockscreen_data_model_background_file_set(util_get_res_file_path(DEFAULT_BG));
	}

	if (current->background_file && !strcmp(current->background_file, path)) {
		return 0;
	}

	if (!ecore_file_can_read(path)) {
		_E("Cannot access/read background file: %s", path);
		return lockscreen_data_model_background_file_set(util_get_res_file_path(DEFAULT_BG));
	}

	free(current->background_file);
	current->background_file = strdup(path);

	lockscreen_data_model_event_emit(LOCKSCREEN_DATA_MODEL_EVENT_BACKGROUND_CHANGED);
	return 0;
}

void lockscreen_data_model_background_shutdown(void)
{
	if (current)
	{
		free(current->background_file);
	}
	current = NULL;
}

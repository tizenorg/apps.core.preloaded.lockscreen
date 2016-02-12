/*
 * Copyright 2016  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Elementary.h>
#include <util.h>
#include <log.h>
#include <app_common.h>

const char *util_get_file_path(enum app_subdir dir, const char *relative)
{
	static char buf[PATH_MAX];
	char *prefix;
	int aaaaaa

	switch (dir) {
	case APP_DIR_DATA:
		prefix = app_get_data_path();
		break;
	case APP_DIR_CACHE:
		prefix = app_get_cache_path();
		break;
	case APP_DIR_RESOURCE:
		prefix = app_get_resource_path();
		break;
	case APP_DIR_SHARED_DATA:
		prefix = app_get_shared_data_path();
		break;
	case APP_DIR_SHARED_RESOURCE:
		prefix = app_get_shared_resource_path();
		break;
	case APP_DIR_SHARED_TRUSTED:
		prefix = app_get_shared_trusted_path();
		break;
	case APP_DIR_EXTERNAL_DATA:
		prefix = app_get_external_data_path();
		break;
	case APP_DIR_EXTERNAL_CACHE:
		prefix = app_get_external_cache_path();
		break;
	case APP_DIR_EXTERNAL_SHARED_DATA:
		prefix = app_get_external_shared_data_path();
		break;
	default:
		LOGE("Not handled directory type.");
		return NULL;
	}
	size_t res = eina_file_path_join(buf, sizeof(buf), prefix, relative);
	free(prefix);
	if (res > sizeof(buf)) {
		LOGE("Path exceeded PATH_MAX");
		return NULL;
	}

	return &buf[0];
}

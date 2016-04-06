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

#include <minicontrol-viewer.h>

#include "log.h"
#include "data_model.h"

static lockscreen_data_model_t *current;

static void _music_player_minicontroller_start_handle(const char *name)
{
	if (!current->active_minicontroller) {
		current->active_minicontroller = eina_stringshare_add(name);
		lockscreen_data_model_event_emit(LOCKSCREEN_DATA_MODEL_EVENT_MINICONTROLLER_CHANGED);
	}
}

static void _music_player_minicontroller_stop_handle(const char *name)
{
	if (current->active_minicontroller && !strcmp(name, current->active_minicontroller)) {
		eina_stringshare_del(current->active_minicontroller);
		current->active_minicontroller = NULL;
		lockscreen_data_model_event_emit(LOCKSCREEN_DATA_MODEL_EVENT_MINICONTROLLER_CHANGED);
	}
}

static void _music_player_minicontroler_event(minicontrol_event_e event, const char *minicontrol_name, bundle *event_arg, void *data)
{
	if (!minicontrol_name)
		return;

	_D("Available minicontroller: %s", minicontrol_name);

	switch (event) {
		case MINICONTROL_EVENT_START:
			_music_player_minicontroller_start_handle(minicontrol_name);
			break;
		case MINICONTROL_EVENT_STOP:
			_music_player_minicontroller_stop_handle(minicontrol_name);
			break;
		default:
			_D("Unahandled minicontroller event: %d for %s", event, minicontrol_name);
			break;
	}
}

int lockscreen_data_model_music_player_init(lockscreen_data_model_t *model)
{
	int ret = minicontrol_viewer_set_event_cb(_music_player_minicontroler_event, NULL);
	if (ret != MINICONTROL_ERROR_NONE) {
		_E("minicontrol_viewer_set_event_cb failed: %s", get_error_message(ret));
		return 1;
	}

	current = model;
	return 0;
}

void lockscreen_data_model_music_player_shutdown(void)
{
	int ret = minicontrol_viewer_unset_event_cb();
	if (ret != MINICONTROL_ERROR_NONE) {
		_E("minicontrol_viewer_unset_event_cb failed: %s", get_error_message(ret));
	}
	eina_stringshare_del(current->active_minicontroller);
}

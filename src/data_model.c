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
#include "data_model_background.h"
#include "data_model_missed_events.h"
#include "data_model_music_player.h"
#include "data_model_sim.h"

#include <Ecore.h>

static lockscreen_data_model_t model;
static int _init_count;
int LOCKSCREEN_DATA_MODEL_EVENT_BACKGROUND_CHANGED;
int LOCKSCREEN_DATA_MODEL_EVENT_MINICONTROLLER_CHANGED;
int LOCKSCREEN_DATA_MODEL_EVENT_LOCK_TYPE_CHANGED;
int LOCKSCREEN_DATA_MODEL_EVENT_SIM_STATUS_CHANGED;
int LOCKSCREEN_DATA_MODEL_EVENT_MISSED_EVENTS_CHANGED;
int LOCKSCREEN_DATA_MODEL_EVENT_TIME_FORMAT_CHANGED;


static void _events_init()
{
	LOCKSCREEN_DATA_MODEL_EVENT_BACKGROUND_CHANGED = ecore_event_type_new();
	LOCKSCREEN_DATA_MODEL_EVENT_MINICONTROLLER_CHANGED = ecore_event_type_new();
	LOCKSCREEN_DATA_MODEL_EVENT_LOCK_TYPE_CHANGED = ecore_event_type_new();
	LOCKSCREEN_DATA_MODEL_EVENT_SIM_STATUS_CHANGED = ecore_event_type_new();
	LOCKSCREEN_DATA_MODEL_EVENT_MISSED_EVENTS_CHANGED = ecore_event_type_new();
	LOCKSCREEN_DATA_MODEL_EVENT_TIME_FORMAT_CHANGED = ecore_event_type_new();
}

int lockscreen_data_model_init()
{
	static bool run_once;

	if (_init_count++)
		return 0;

	ecore_init();

	if (!run_once)
	{
		_events_init();
		run_once = true;
	}

	int ret = lockscreen_data_model_background_init(&model);
	if (ret) goto battery_shutdown;
	ret = lockscreen_data_model_sim_init(&model);
	if (ret) goto background_shutdown;
	ret = lockscreen_data_model_missed_events_init(&model);
	if (ret) goto sim_shutdown;
	ret = lockscreen_data_model_music_player_init(&model);
	if (ret) goto missed_shutdown;
	ret = lockscreen_data_model_time_init(&model);
	if (ret) goto display_shutdown;

	return 0;

display_shutdown:
camera_shutdown:
music_shutdown:
	lockscreen_data_model_music_player_shutdown();
missed_shutdown:
	lockscreen_data_model_missed_events_shutdown();
sim_shutdown:
	lockscreen_data_model_sim_shutdown();
background_shutdown:
	lockscreen_data_model_background_shutdown();
battery_shutdown:

	ecore_shutdown();

	_E("Data model initialization failed.");
	return -1;
}

const lockscreen_data_model_t *lockscreen_data_model_get_model(void)
{
	return &model;
}

int lockscreen_data_model_shutdown()
{
	if (_init_count)
	{
		if (--_init_count) return 0;

		lockscreen_data_model_background_shutdown();
		lockscreen_data_model_sim_shutdown();
		lockscreen_data_model_missed_events_shutdown();
		lockscreen_data_model_music_player_shutdown();

		ecore_shutdown();
	}

	return 0;
}

static void dummy_end(void *user_data, void *func_data)
{
}

int lockscreen_data_model_event_emit(int event)
{
	return ecore_event_add(event, &model, dummy_end, NULL) ? 0 : -1;
}

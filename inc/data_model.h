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

#ifndef _LOCKSCREEN_DATA_MODEL_H_
#define _LOCKSCREEN_DATA_MODEL_H_

#include <Eina.h>
#include <stdbool.h>

typedef struct {
	struct {
		bool is_connected; /* true if charger is conencted */
		bool is_charging; /* true if charger is charing */
		int level; /* Battery level 0-100 */
	} battery;
	char *background_file;
	const char *active_minicontroller;
	bool lcd_off;
	int lock_type;
	struct {
		char *text; /* for two sims */
	} sim[2];
	bool camera_on; /* true if camera shortcut should be enabled */
	Eina_List *missed_events; /* List of missed_event_t structs */
	struct {
		char *locale;
		bool use24hformat;
	} time;
} lockscreen_data_model_t;


extern int LOCKSCREEN_DATA_MODEL_EVENT_BATTERY_CHANGED;
extern int LOCKSCREEN_DATA_MODEL_EVENT_BACKGROUND_CHANGED;
extern int LOCKSCREEN_DATA_MODEL_EVENT_MINICONTROLLER_CHANGED;
extern int LOCKSCREEN_DATA_MODEL_EVENT_LCD_STATUS_CHANGED;
extern int LOCKSCREEN_DATA_MODEL_EVENT_LOCK_TYPE_CHANGED;
extern int LOCKSCREEN_DATA_MODEL_EVENT_SIM_STATUS_CHANGED;
extern int LOCKSCREEN_DATA_MODEL_EVENT_MISSED_EVENTS_CHANGED;
extern int LOCKSCREEN_DATA_MODEL_EVENT_CAMERA_STATUS_CHANGED;
extern int LOCKSCREEN_DATA_MODEL_EVENT_TIME_FORMAT_CHANGED;

int lockscreen_data_model_init();

const lockscreen_data_model_t *lockscreen_data_model_get_model(void);

int lockscreen_data_model_shutdown();

int lockscreen_data_model_event_emit(int event);

#include "data_model_background.h"
#include "data_model_battery.h"
#include "data_model_camera.h"
#include "data_model_missed_events.h"
#include "data_model_music_player.h"
#include "data_model_sim.h"
#include "data_model_display.h"
#include "data_model_missed_events.h"
#include "data_model_time.h"

#endif

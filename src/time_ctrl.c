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
#include "time_ctrl.h"
#include "data_model.h"
#include "main_view.h"

#include <Ecore.h>
#include <time.h>

static Ecore_Event_Handler *handler, *display_handler;
static Ecore_Timer *update_timer;

static void _time_update(void)
{
	const lockscreen_data_model_t *model = lockscreen_data_model_get_model();

	lockscreen_main_view_time_set(model->time.locale, model->time.timezone, model->time.use24hformat, time(NULL));
}

static Eina_Bool _timer_cb(void *data)
{
	_time_update();
	ecore_timer_interval_set(update_timer, 60.0);
	return ECORE_CALLBACK_RENEW;
}

static void _time_spawn_timer(void)
{
	time_t tt;
	struct tm st;

	tt = time(NULL);
	localtime_r(&tt, &st);

	update_timer = ecore_timer_add(60 - st.tm_sec, _timer_cb, NULL);
}

static Eina_Bool _time_changed(void *data, int event, void *event_info)
{
	_time_update();
	_time_spawn_timer();
	return EINA_TRUE;
}

static Eina_Bool _lcd_status_changed(void *data, int event, void *event_info)
{
	const lockscreen_data_model_t *model = lockscreen_data_model_get_model();

	if (model->lcd_off) {
		if (update_timer) ecore_timer_freeze(update_timer);
	}
	else {
		_time_update();
		_time_spawn_timer();
	}
	return EINA_TRUE;
}

void lockscreen_time_ctrl_init(void)
{
	handler = ecore_event_handler_add(LOCKSCREEN_DATA_MODEL_EVENT_TIME_FORMAT_CHANGED, _time_changed, NULL);
	if (!handler)
		FATAL("ecore_event_handler_add failed on LOCKSCREEN_DATA_MODEL_EVENT_TIME_FORMAT_CHANGED event");
	display_handler = ecore_event_handler_add(LOCKSCREEN_DATA_MODEL_EVENT_LCD_STATUS_CHANGED, _lcd_status_changed, NULL);
	if (!display_handler)
		FATAL("ecore_event_handler_add failed on LOCKSCREEN_DATA_MODEL_EVENT_LCD_STATUS_CHANGED event");
	_time_update();
	_time_spawn_timer();
}

void lockscreen_time_ctrl_fini(void)
{
	ecore_event_handler_del(handler);
	ecore_event_handler_del(display_handler);
}


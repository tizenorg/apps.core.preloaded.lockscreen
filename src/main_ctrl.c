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
#include "main_ctrl.h"
#include "main_view.h"
#include "window.h"
#include "battery_ctrl.h"
#include "background.h"
#include "camera_ctrl.h"
#include "time_format_ctrl.h"
#include "util.h"
#include "sim_ctrl.h"

#include <Elementary.h>
#include <efl_extension.h>
#include <app.h>

static Evas_Object *win;
static Evas_Object *view;

static void _view_unlocked(void *data, Evas_Object *obj, void *event)
{
	ui_app_exit();
}

static void _swipe_finished(void *data, Evas_Object *obj, void *event)
{
	/* When swipe finished play unlock animation and exit */
	evas_object_smart_callback_add(obj, SIGNAL_UNLOCK_ANIMATION_FINISHED, _view_unlocked, NULL);
	lockscreen_main_view_unlock(obj);
}

static void _back_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	util_feedback_tap_play();
}

int lockscreen_main_ctrl_init(void)
{
	win = lockscreen_window_create();
	if (!win)
		FATAL("elm_win_add failed.");

	view = lockscreen_main_view_create(win);
	if (!view)
		FATAL("lockscreen_main_view_create failed.");

	if (lockscreen_background_init()) {
		FATAL("lockscreen_background_init failed. Background changes will not be available");
	} else {
		if (!lockscreen_main_view_background_set(view, LOCKSCREEN_BACKGROUND_TYPE_DEFAULT, lockscreen_background_file_get()))
			FATAL("lockscreen_main_view_background_image_set failed");
	}

	lockscreen_window_content_set(view);
	evas_object_smart_callback_add(view, SIGNAL_SWIPE_GESTURE_FINISHED, _swipe_finished, NULL);
	eext_object_event_callback_add(win, EEXT_CALLBACK_BACK, _back_key_cb, NULL);

	// init subcontrollers
	if (lock_battery_ctrl_init(view))
		FATAL("lock_battery_ctrl_init failed. Battery information will not be available");

	if (lockscreen_camera_ctrl_init(view))
		FATAL("lockscreen_camera_ctrl_init failed. Camera quickshot will not be available");

	if (lockscreen_time_format_ctrl_init(view))
		FATAL("lockscreen_time_format_ctrl_init failed. Time format changes will not be available");

	if (lockscreen_sim_ctrl_init(view))
		FATAL("lockscreen_sim_ctrl_init failed. Sim PLMN updates will not be available");

	return 0;
}

void lockscreen_main_ctrl_shutdown(void)
{
	lockscreen_background_shutdown();
}

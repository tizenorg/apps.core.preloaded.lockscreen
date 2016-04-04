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
#include "data_model.h"
#include "battery_ctrl.h"
#include "camera_ctrl.h"

#include <Elementary.h>

static Evas_Object *win;
static Evas_Object *view;

static void _layout_unlocked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	ui_app_exit();
}

void lockscreen_main_ctrl_init(void)
{
	if (lockscreen_data_model_init())
		FATAL("lockscreen_data_model_init failed.");

	const lockscreen_data_model_t *model = lockscreen_data_model_get_model();
	if (!model)
		FATAL("lockscreen_data_model_get_model failed.");

	win = lockscreen_window_create();
	if (!win)
		FATAL("elm_win_add failed.");

	view = lockscreen_main_view_create(win);
	if (!view)
		FATAL("lockscreen_main_view_create failed.");

	if (!lockscreen_main_view_background_set(LOCKSCREEN_BACKGROUND_TYPE_DEFAULT, model->background_file))
		FATAL("lockscreen_main_view_background_image_set failed");

	lockscreen_window_content_set(view);

	lockscreen_main_view_unlock_signal_add(_layout_unlocked, NULL);

	// init subcontrollers
	lock_battery_ctrl_init();
	lockscreen_camera_ctrl_init();
}

void lockscreen_main_ctrl_shutdown(void)
{
	lockscreen_main_view_unlock_signal_del(_layout_unlocked);
}

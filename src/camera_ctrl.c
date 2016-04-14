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

#include <Ecore.h>

#include "log.h"
#include "camera_ctrl.h"
#include "camera.h"
#include "main_view.h"

static Ecore_Event_Handler *handler;
static Evas_Object *main_view;

static void _camera_view_update()
{
	if (lockscreen_camera_is_on())
		lockscreen_main_view_camera_show(main_view);
	else
		lockscreen_main_view_camera_hide(main_view);
}

static Eina_Bool _cam_status_changed(void *data, int event, void *event_info)
{
	_camera_view_update();
	return EINA_TRUE;
}

static void _camera_clicked(void *data, Evas_Object *obj, void *event)
{
	lockscreen_camera_activate();
}

int lockscreen_camera_ctrl_init(Evas_Object *view)
{
	if (lockscreen_camera_init()) {
		_E("lockscreen_camera_init failed");
		return 1;
	}

	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_CAMERA_STATUS_CHANGED, _cam_status_changed, NULL);
	if (!handler)
		FATAL("ecore_event_handler_add failed on LOCKSCREEN_EVENT_BATTERY_CHANGED event");
	main_view = view;
	_camera_view_update();

	evas_object_smart_callback_add(view, SIGNAL_CAMERA_SELECTED, _camera_clicked, NULL);
	return 0;
}

void lockscreen_camera_ctrl_fini(void)
{
	ecore_event_handler_del(handler);
	evas_object_smart_callback_del(main_view, SIGNAL_CAMERA_SELECTED, _camera_clicked);
	lockscreen_camera_shutdown();
}

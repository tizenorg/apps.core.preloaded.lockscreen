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

static void _camera_view_update()
{
	if (lockscreen_camera_is_on())
		lockscreen_main_view_camera_show();
	else
		lockscreen_main_view_camera_hide();
}

static Eina_Bool _cam_status_changed(void *data, int event, void *event_info)
{
	_camera_view_update();
	return EINA_TRUE;
}

static void _camera_clicked(void)
{
	lockscreen_camera_activate();
}

void lockscreen_camera_ctrl_init(void)
{
	if (lockscreen_camera_init()) {
		_E("lockscreen_camera_init failed");
		return;
	}

	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_CAMERA_STATUS_CHANGED, _cam_status_changed, NULL);
	if (!handler)
		FATAL("ecore_event_handler_add failed on LOCKSCREEN_EVENT_BATTERY_CHANGED event");
	_camera_view_update();

	lockscreen_main_view_camera_clicked_signal_add(_camera_clicked);
}

void lockscreen_camera_ctrl_fini(void)
{
	ecore_event_handler_del(handler);
	lockscreen_main_view_camera_clicked_signal_del();
	lockscreen_camera_shutdown();
}

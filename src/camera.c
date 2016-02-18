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

#include "camera.h"
#include "log.h"

#include <app_control.h>
#define KEY_DISPLAY_OVER_LOCKSCREEN "http://tizen.org/lock/window/above"

static void _app_control_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	switch (result) {
		case APP_CONTROL_RESULT_APP_STARTED:
		case APP_CONTROL_RESULT_SUCCEEDED:
			_D("Camera application launch succcessed.");
			break;
		case APP_CONTROL_RESULT_FAILED:
		case APP_CONTROL_RESULT_CANCELED:
			_D("Camera application launch failed.");
			break;
	}
	lockscreen_lcd_off_timer_set();
}

lock_error_e lock_camera_run(void)
{
	app_control_h app_ctr;

	int err = app_control_create(&app_ctr);
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_create failed: %s", get_error_message(err));
		return LOCK_ERROR_FAIL;
	}

	err = app_control_set_launch_mode(app_ctr, APP_CONTROL_LAUNCH_MODE_GROUP);
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_set_launch_mode failed: %s", get_error_message(err));
		app_control_destroy(app_ctr);
		return LOCK_ERROR_FAIL;
	}

	/* Send a hint to camera-app to display itself over lockscreen */
	err = app_control_add_extra_data(app_ctr, KEY_DISPLAY_OVER_LOCKSCREEN, "on");
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_add_extra_data failed: %s", get_error_message(err));
		app_control_destroy(app_ctr);
		return LOCK_ERROR_FAIL;
	}

	/* Send a hint to camera-app to display itself in secure mode*/
	err = app_control_add_extra_data(app_ctr, "secure_mode", "true");
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_add_extra_data failed: %s", get_error_message(err));
		app_control_destroy(app_ctr);
		return LOCK_ERROR_FAIL;
	}

	err = app_control_set_app_id(app_ctr, "org.tizen.camera-app");
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_set_app_id failed: %s", get_error_message(err));
		app_control_destroy(app_ctr);
		return LOCK_ERROR_FAIL;
	}

	err = app_control_send_launch_request(app_ctr, _app_control_reply_cb, NULL);
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_send_launch_request failed: %s", get_error_message(err));
		app_control_destroy(app_ctr);
		return LOCK_ERROR_FAIL;
	}

	_D("Launch request send for %s", APP_CONTROL_OPERATION_CREATE_CONTENT);
	app_control_destroy(app_ctr);
	lockscreen_lcd_off_timer_unset();

	return LOCK_ERROR_OK;
}

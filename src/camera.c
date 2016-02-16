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

static app_control_h app_ctr;

lock_error_e lock_camera_init(void)
{
	return LOCK_ERROR_OK;
}

lock_error_e lock_camera_fini(void)
{
	if (app_ctr) {
		app_control_destroy(app_ctr);
		app_ctr = NULL;
	}

	return LOCK_ERROR_OK;
}

static void _app_control_send_request(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	switch (result) {
		case APP_CONTROL_RESULT_APP_STARTED:
		case APP_CONTROL_RESULT_SUCCEEDED:
			_D("Launched request for image capture successed.");
			break;
		case APP_CONTROL_RESULT_FAILED:
		case APP_CONTROL_RESULT_CANCELED:
		default:
			_E("Launched request for image capture failed.");
			break;
	}

	/* Clean handle */
	lock_camera_fini();
}

lock_error_e lock_camera_run(void)
{
	if (app_ctr) {
		_W("App control request already launched");
		return LOCK_ERROR_OK;
	}

	int err = app_control_create(&app_ctr);
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_create failed: %s", get_error_message(err));
		return LOCK_ERROR_FAIL;
	}

	err = app_control_set_operation(app_ctr, APP_CONTROL_OPERATION_IMAGE_CAPTURE);
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_set_operation failed: %s", get_error_message(err));
		lock_camera_fini();
		return LOCK_ERROR_FAIL;
	}

	/* Force to launch only trusted camera application */
	err = app_control_set_app_id(app_ctr, "org.tizen.camera-app");
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_set_app_id failed: %s", get_error_message(err));
		lock_camera_fini();
		return LOCK_ERROR_FAIL;
	}

	//FIXME
	//How to inform that org.tizen.camera-app should be launched in "safe" mode?

	err = app_control_send_launch_request(app_ctr, _app_control_send_request, NULL);
	if (err != APP_CONTROL_ERROR_NONE) {
		_E("app_control_send_launch_request failed: %s", get_error_message(err));
		lock_camera_fini();
		return LOCK_ERROR_FAIL;
	}

	_D("Launch request send for %s", APP_CONTROL_OPERATION_IMAGE_CAPTURE);

	return LOCK_ERROR_OK;
}

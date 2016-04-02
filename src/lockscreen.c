/*
 * Copyright (c) 2009-2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include <stdio.h>
#include <stdlib.h>

#include <Evas.h>
#include <Elementary.h>
#include <Eina.h>
#include <efl_extension.h>

#include <app.h>
#include <feedback.h>

#include "lockscreen.h"
#include "log.h"
#include "property.h"
#include "window.h"
#include "background_view.h"
#include "default_lock.h"
#include "lock_time.h"
#include "data_model.h"
#include <device/display.h>
#include <device/callback.h>

#define LOCK_CONTROL_TYPE_KEY "lock_type"
#define LOCK_CONTROL_TYPE_VALUE_RECOVERY "recovery_lock"
#define LOCK_CONTROL_KEY "lock_op"
#define LOCK_CONTROL_VALUE_START_READY "start_ready"

#define LOCK_LCD_OFF_TIMEOUT_TIME 10

static struct _s_info {
	Ecore_Timer *lcd_off_timer;
	int lock_type;
	int lcd_off_count;

} s_info = {
	.lcd_off_timer = NULL,
	.lock_type = 0,
	.lcd_off_count = 0,
};

int lockscreen_setting_lock_type_get(void)
{
	return s_info.lock_type;
}

Ecore_Timer *lockscreen_lcd_off_timer_get(void)
{
	return s_info.lcd_off_timer;
}

int lockscreen_lcd_off_count_get(void)
{
	return s_info.lcd_off_count;
}

void lockscreen_feedback_tap_play(void)
{
	if (!lock_property_sound_touch_get()) {
		return;
	}

	feedback_play_type(FEEDBACK_TYPE_SOUND, FEEDBACK_PATTERN_TAP);
}

static Eina_Bool _lcd_off_timer_cb(void *data)
{
	int ret = 0;

	ret = device_display_change_state(DISPLAY_STATE_SCREEN_OFF);
	if (ret != DEVICE_ERROR_NONE) {
		_E("Failed to change display state : %d", ret);
	} else {
		_I("Display off : %dsec", LOCK_LCD_OFF_TIMEOUT_TIME);
	}

	return ECORE_CALLBACK_CANCEL;
}

void lockscreen_lcd_off_timer_set(void)
{
	if (s_info.lcd_off_timer) {
		ecore_timer_del(s_info.lcd_off_timer);
		s_info.lcd_off_timer = NULL;
	}

	s_info.lcd_off_timer = ecore_timer_add(LOCK_LCD_OFF_TIMEOUT_TIME, _lcd_off_timer_cb, NULL);
}

void lockscreen_lcd_off_timer_reset(void)
{
	if (s_info.lcd_off_timer) {
		ecore_timer_reset(s_info.lcd_off_timer);
	}
}

void lockscreen_lcd_off_timer_unset(void)
{
	if (s_info.lcd_off_timer) {
		ecore_timer_del(s_info.lcd_off_timer);
		s_info.lcd_off_timer = NULL;
		_I("unset lcd off timer");
	}
}

void lockscreen_lcd_off_count_raise(void)
{
	if (s_info.lcd_off_count < 3) {
		_D("count for lcd off timer : %d", s_info.lcd_off_count);
		lockscreen_lcd_off_timer_reset();
		s_info.lcd_off_count++;
	}
}

void lockscreen_lcd_off_count_reset(void)
{
	_D("lcd off count reset : %d -> 0", s_info.lcd_off_count);
	s_info.lcd_off_count = 0;
}

static Eina_Bool _lock_idler_cb(void *data)
{
	lockscreen_main_ctrl_init();

	/*
	if (LOCK_ERROR_OK != lock_default_lock_init()) {
		_E("Failed to initialize default lockscreen");
		return ECORE_CALLBACK_CANCEL;
	}
	*/

	/* register callback func. : key sound, touch sound, rotation */
	//lock_property_register(NULL);

	feedback_initialize();

	lockscreen_lcd_off_timer_set();

#if 0
	/* set rotation changed cb */
	if (elm_win_wm_rotation_supported_get(win)) {
		int rots[4] = { 0, };
		elm_win_wm_rotation_available_rotations_set(win, rots, 0);
	}
#endif

	return ECORE_CALLBACK_CANCEL;
}

static void _back_key_cb(void *data, Evas_Object *obj, void *event_info)
{
	_D("%s", __func__);

	lockscreen_feedback_tap_play();
}

static void _display_status_changed(device_callback_e type, void *value, void *user_data)
{
	if (type != DEVICE_CALLBACK_DISPLAY_STATE)
		return;

	display_state_e state = (display_state_e)value;

	if (state == DISPLAY_STATE_NORMAL) {
		_I("Display on");
		lock_time_resume();
		lockscreen_lcd_off_timer_set();
	}
	else if (state == DISPLAY_STATE_SCREEN_OFF) {
		_I("Display off");
		lock_time_pause();
		lockscreen_lcd_off_timer_unset();
		lockscreen_lcd_off_count_reset();
	}
}

bool _create_app(void *data)
{
	_D("%s", __func__);

	elm_config_accel_preference_set("opengl");

	_D("base scale : %f", elm_app_base_scale_get());

// FIXME
#if 0
	/* Get lockscreen type */
	int ret = lock_property_get_int(PROPERTY_TYPE_VCONFKEY, VCONFKEY_SETAPPL_SCREEN_LOCK_TYPE_INT, &locktype);
	if (ret != LOCK_ERROR_OK) {
		_E("Failed to get lockscreen type. Set default lockscreen.");
		locktype = SETTING_SCREEN_LOCK_TYPE_SWIPE;
	}
	_D("lockscreen type : %d", locktype);
	s_info.lock_type = locktype;
#endif
	s_info.lock_type = 1;

	/* Create lockscreen window */
	//win = lock_window_create(locktype);
	//retv_if(!win, false);

	/* Create lockscreen BG */
	//bg = lock_background_view_bg_create(win);
	//if (!bg) {
	//	_E("Failed to create BG");
	//}

	/* Register on display on/off events */
	int ret = device_add_callback(DEVICE_CALLBACK_DISPLAY_STATE, _display_status_changed, NULL);
	if (ret != DEVICE_ERROR_NONE) {
		_E("Failed to register display state changed callback.");
	}

	//evas_object_show(win);

	ecore_idler_add(_lock_idler_cb, NULL);

	//eext_object_event_callback_add(win, EEXT_CALLBACK_BACK, _back_key_cb, NULL);

	return true;
}

void _terminate_app(void *data)
{
	_D("%s", __func__);

	lock_default_lock_fini();

	//lock_property_unregister();
	feedback_deinitialize();
}

int main(int argc, char *argv[])
{
	int ret = 0;

	ui_app_lifecycle_callback_s lifecycle_callback = {0,};

	lifecycle_callback.create = _create_app;
	lifecycle_callback.terminate = _terminate_app;

	ret = ui_app_main(argc, argv, &lifecycle_callback, NULL);
	if (ret != APP_ERROR_NONE) {
		_E("ui_app_main failed: %s", get_error_message(ret));
	}

	return ret;
}

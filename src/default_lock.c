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

#include <Elementary.h>
#include <efl_extension.h>
#include <app.h>
#include <minicontrol-viewer.h>
#include <bundle.h>

#include "lockscreen.h"
#include "log.h"
#include "default_lock.h"
#include "property.h"
#include "window.h"
#include "background_view.h"
#include "battery_ctrl.h"
#include "lock_time.h"
#include "sim_state.h"
#include "util.h"
#include "contextual_view.h"

static struct _s_info {
	Evas_Object *conformant;
	Evas_Object *layout;
	Evas_Object *swipe_layout;
	Evas_Object *gesture_layer;

	int lcd_off_count;

	lock_exit_state_e exit_state;
} s_info = {
	.conformant = NULL,
	.layout= NULL,
	.swipe_layout = NULL,

	.exit_state = LOCK_EXIT_STATE_NORMAL,
};

Evas_Object *lock_default_conformant_get(void)
{
	return s_info.conformant;
}

Evas_Object *lock_default_lock_layout_get(void)
{
	return s_info.layout;
}

Evas_Object *lock_default_swipe_layout_get(void)
{
	return s_info.swipe_layout;
}

void _default_lock_hw_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	_I("%s", __func__);
}

static void _vi_effect_end_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	_D("%s", __func__);

	ui_app_exit();
}

static lock_error_e _unlock_panel_create(void)
{
	retv_if(!s_info.swipe_layout, LOCK_ERROR_FAIL);

	elm_object_signal_callback_add(s_info.swipe_layout, "vi_effect_end", "vi_clipper", _vi_effect_end_cb, NULL);

	return LOCK_ERROR_OK;
}

static Evas_Event_Flags _swipe_state_start(void *data, void *event_info)
{
	_D("Swipe gesture start");

	lockscreen_lcd_off_count_raise();

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _swipe_state_end(void *data, void *event_info)
{
	_D("Swipe gesture end");
	s_info.exit_state = LOCK_EXIT_STATE_EXIT;
	elm_object_signal_emit(s_info.swipe_layout, "vi_effect", "padding.top");
	elm_object_signal_emit(s_info.layout, "vi_effect", "vi_clipper");
	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _swipe_state_abort(void *data, void *event_info)
{
	_D("Swipe gesture abort");
	s_info.exit_state = LOCK_EXIT_STATE_NORMAL;
	elm_object_signal_emit(s_info.swipe_layout, "vi_effect_stop", "padding.top");
	elm_object_signal_emit(s_info.layout, "vi_effect_stop", "vi_clipper");
	elm_object_signal_emit(s_info.swipe_layout, "show,txt,plmn", "txt.plmn");
	return EVAS_EVENT_FLAG_NONE;
}

static void _camera_icon_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
#if 0
	if (lock_camera_run() != LOCK_ERROR_OK) {
		_E("Unable to launch camera application.");
		return;
	}
#endif
}

static Evas_Object *_swipe_layout_create(Evas_Object *parent)
{
	Evas_Object *swipe_layout = NULL;
	Evas_Object *edje;

	retv_if(!parent, NULL);

	swipe_layout = elm_layout_add(parent);
	retv_if(!swipe_layout, NULL);

	if (!elm_layout_file_set(swipe_layout, util_get_res_file_path(LOCK_EDJE_FILE), "swipe-lock")) {
		_E("Failed to set edje file for swipe lock");
		goto ERROR;
	}

	evas_object_size_hint_weight_set(swipe_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(swipe_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(swipe_layout);
	s_info.swipe_layout = swipe_layout;

	edje = elm_layout_add(swipe_layout);
	if (!edje)
		_E("Unable to get edje file from elm_layout");

	if (!elm_layout_file_set(edje, util_get_res_file_path(LOCK_EDJE_FILE), "camera-layout"))
		_E("Unable to set camara view layout");

	elm_object_signal_callback_add(edje, "camera,icon,clicked", "img.camera", _camera_icon_clicked, NULL);
	elm_object_part_content_set(swipe_layout, "sw.camera", edje);

	/* initialize time & date information */
	lock_time_init();

	/* initialize PLMN-SPN information */
	if (LOCK_ERROR_OK != lock_sim_state_init()) {
		_E("Failed to initialize sim state");
	}

	return swipe_layout;

ERROR:
	_E("Failed to create swipe layout");

	if(swipe_layout) {
		evas_object_del(swipe_layout);
		swipe_layout = NULL;
	}

	return NULL;
}

static Evas_Object *_layout_create(void)
{
	Evas_Object *layout = NULL;
	Evas_Object *swipe_layout = NULL;
	Evas_Object *win = NULL;
	Evas_Object *layer;

	win = lock_window_win_get();
	retv_if(!win, NULL);

	layout = elm_layout_add(win);
	retv_if(!layout, NULL);

	evas_object_show(layout);

	if (!elm_layout_file_set(layout, util_get_res_file_path(LOCK_EDJE_FILE), "lockscreen")) {
		_E("Failed to set edje file");
		goto ERROR;
	}

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	swipe_layout = _swipe_layout_create(layout);
	if (!swipe_layout) {
		_E("Failed to create swipe layout");
		goto ERROR;
	}

	/* intialize gesture layer */
	layer = elm_gesture_layer_add(win);
	elm_gesture_layer_cb_set(layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_START, _swipe_state_start, NULL);
	elm_gesture_layer_cb_set(layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_END, _swipe_state_end, NULL);
	elm_gesture_layer_cb_set(layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_ABORT, _swipe_state_abort, NULL);
	elm_gesture_layer_hold_events_set(layer, EINA_TRUE);
	elm_gesture_layer_attach(layer, layout);
	evas_object_show(layer);
	s_info.gesture_layer = layer;

	elm_object_part_content_set(layout, "sw.swipe_layout", swipe_layout);
	if (!elm_object_part_content_get(layout, "sw.swipe_layout")) {
		_E("Failed to set swipe layout");
		goto ERROR;
	}

	elm_win_resize_object_add(win, layout);

	return layout;

ERROR:
	_E("Failed to create layout");

	if (layout) {
		evas_object_del(layout);
		layout = NULL;
	}

	if (swipe_layout) {
		evas_object_del(swipe_layout);
		swipe_layout = NULL;
	}

	return NULL;
}

static Evas_Object *_comformant_create(void)
{
	Evas_Object *conformant = NULL;
	Evas_Object *win = NULL;

	win = lock_window_win_get();
	retv_if(!win, NULL);

	conformant = elm_conformant_add(win);
	retv_if(!conformant, NULL);

	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, conformant);

	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	elm_object_signal_emit(conformant, "elm,state,indicator,overlap", "elm");

	evas_object_show(conformant);

	return conformant;
}

#if 0
static void _context_changed(bool enabled)
{
	_D("Context chagned");

	Evas_Object *swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	if (!enabled) return;

	/*
	Evas_Object *contextual_event_layout = elm_object_part_content_get(swipe_layout, "sw.contextual_event");
	if (!contextual_event_layout) {
		_D("create contextual event layout");
		contextual_event_layout = lock_contextual_view_layout_create(swipe_layout);
		retv_if(!contextual_event_layout, LOCK_ERROR_FAIL);

		elm_object_part_content_set(swipe_layout, "sw.contextual_event", contextual_event_layout);
	}
	*/
}
#endif

lock_error_e lock_default_lock_init(void)
{
	Evas_Object *conformant = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *bg = NULL;

	int ret = 0;

	layout = _layout_create();
	goto_if(!layout, ERROR);

	s_info.layout = layout;

	conformant = _comformant_create();
	goto_if(!conformant, ERROR);
	s_info.conformant = conformant;

	eext_object_event_callback_add(layout, EEXT_CALLBACK_BACK, _default_lock_hw_back_cb, NULL);

	bg = lock_background_view_bg_get();
	if (!bg) {
		_E("Failed to get BG");
	} else {
		elm_object_part_content_set(layout, "sw.bg", bg);
	}

	ret = _unlock_panel_create();
	goto_if(LOCK_ERROR_OK != ret, ERROR);

	//lock_contextual_view_init();
	//lock_contextual_view_state_changed_cb_set(_context_changed, NULL);

	return LOCK_ERROR_OK;

ERROR:
	_E("Failed to initialize default lock");

	if (conformant) {
		evas_object_del(conformant);
		conformant = NULL;
	}

	if (layout) {
		evas_object_del(layout);
		layout = NULL;
	}

	return LOCK_ERROR_FAIL;
}

void lock_default_lock_fini(void)
{
	lock_sim_state_deinit();
	lock_time_fini();
	lock_background_view_bg_del();

	if (s_info.swipe_layout) {
		evas_object_del(s_info.swipe_layout);
		s_info.swipe_layout = NULL;
	}

	if (s_info.conformant) {
		evas_object_del(s_info.conformant);
		s_info.conformant = NULL;
	}

	if (s_info.layout) {
		eext_object_event_callback_del(s_info.layout, EEXT_CALLBACK_BACK, _default_lock_hw_back_cb);
		evas_object_del(s_info.layout);
		s_info.layout = NULL;
	}

	if (s_info.gesture_layer) {
		evas_object_del(s_info.gesture_layer);
		s_info.gesture_layer = NULL;
	}

	lock_contextual_view_shutdown();
}

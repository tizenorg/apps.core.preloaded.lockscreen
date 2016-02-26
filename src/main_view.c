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

#include "main_view.h"
#include "util.h"
#include "log.h"
#include "lockscreen.h"

#include <Elementary.h>

struct {
	Evas_Object *layout;
	Evas_Object *swipe_layout;
	Evas_Object *bg;
	Evas_Object *gesture_layer;
	Evas_Object *cam_layout;
} view;

static Evas_Object *_swipe_layout_create(Evas_Object *parent)
{
	Evas_Object *swipe_layout = NULL;

	retv_if(!parent, NULL);

	swipe_layout = elm_layout_add(parent);

	if (!elm_layout_file_set(swipe_layout, util_get_res_file_path(LOCK_EDJE_FILE), "swipe-lock")) {
		_E("elm_layout_file_set failed.");
		evas_object_del(swipe_layout);
		return NULL;
	}

	evas_object_size_hint_weight_set(swipe_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(swipe_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(swipe_layout);

	return swipe_layout;
}

bool lockscreen_main_view_camera_show(void)
{
	Evas_Object *cam_ly = elm_layout_add(view.swipe_layout);
	if (!elm_layout_file_set(cam_ly, util_get_res_file_path(LOCK_EDJE_FILE), "camera-layout")) {
		_E("elm_layout_file_set failed");
		return false;
	}
	elm_object_part_content_set(view.swipe_layout, "sw.camera", cam_ly);
	evas_object_show(cam_ly);
	view.cam_layout = cam_ly;

	return true;
}

void lockscreen_main_view_camera_hide()
{
	elm_object_part_content_set(view.swipe_layout, "sw.camera", NULL);
	view.cam_layout = NULL;
}

void lockscreen_main_view_camera_clicked_signal_add(Edje_Signal_Cb cb, void *data)
{
	elm_object_signal_callback_add(view.cam_layout, "camera,icon,clicked", "img.camera", cb, data);
}

static Evas_Event_Flags _swipe_state_start(void *data, void *event_info)
{
	_D("Swipe gesture start");

	//lockscreen_lcd_off_count_raise();

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _swipe_state_end(void *data, void *event_info)
{
	_D("Swipe gesture end");
	//s_info.exit_state = LOCK_EXIT_STATE_EXIT;
	elm_object_signal_emit(view.swipe_layout, "vi_effect", "padding.top");
	elm_object_signal_emit(view.layout, "vi_effect", "vi_clipper");
	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _swipe_state_abort(void *data, void *event_info)
{
	_D("Swipe gesture abort");
	//s_info.exit_state = LOCK_EXIT_STATE_NORMAL;
	elm_object_signal_emit(view.swipe_layout, "vi_effect_stop", "padding.top");
	elm_object_signal_emit(view.layout, "vi_effect_stop", "vi_clipper");
	elm_object_signal_emit(view.swipe_layout, "show,txt,plmn", "txt.plmn");
	return EVAS_EVENT_FLAG_NONE;
}

Evas_Object *_gesture_layer_create(Evas_Object *parent)
{
	/* intialize gesture layer */
	Evas_Object *layer = elm_gesture_layer_add(parent);
	elm_gesture_layer_cb_set(layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_START, _swipe_state_start, NULL);
	elm_gesture_layer_cb_set(layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_END, _swipe_state_end, NULL);
	elm_gesture_layer_cb_set(layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_ABORT, _swipe_state_abort, NULL);
	elm_gesture_layer_hold_events_set(layer, EINA_TRUE);
	elm_gesture_layer_attach(layer, parent);

	evas_object_show(layer);
	return layer;
}

Evas_Object *lockscreen_main_view_create(Evas_Object *parent)
{
	view.layout = elm_layout_add(parent);

	if (!elm_layout_file_set(view.layout, util_get_res_file_path(LOCK_EDJE_FILE), "lockscreen")) {
		_E("Failed to set edje file for main view.");
		return NULL;
	}

	evas_object_show(view.layout);
	evas_object_size_hint_weight_set(view.layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(view.layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	view.swipe_layout = _swipe_layout_create(view.layout);
	if (!view.swipe_layout) {
		evas_object_del(view.layout);
		return NULL;
	}

	view.bg = elm_bg_add(view.layout);
	elm_bg_option_set(view.bg, ELM_BG_OPTION_SCALE);
	evas_object_size_hint_weight_set(view.bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(view.bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(view.bg);

	view.gesture_layer = _gesture_layer_create(view.layout);

	elm_object_part_content_set(view.layout, "sw.swipe_layout", view.swipe_layout);

	return view.layout;
}

void lockscreen_main_view_destroy()
{
	evas_object_del(view.layout);
}

bool lockscreen_main_view_background_image_set(lockscreen_main_view_background_type type, const char *file)
{
	if (!elm_bg_file_set(view.bg, file, NULL)) {
		_E("elm_bg_file_set failed: %s", file);
		return false;
	}

	switch (type) {
		case LOCKSCREEN_BACKGROUND_TYPE_DEFAULT:
			elm_layout_signal_emit(view.layout, "music_off", "bg");
			break;
		case LOCKSCREEN_BACKGROUND_TYPE_ALBUM_ART:
			elm_layout_signal_emit(view.layout, "music_on", "bg");
			break;
		default:
			_E("Unhandled background type");
	}

	return true;
}

void lockscreen_main_view_battery_status_text_set(const char *battery)
{
	if (battery) {
		elm_object_part_text_set(view.swipe_layout, "txt.battery", battery);
		elm_object_signal_emit(view.swipe_layout, "show,txt,battery", "txt.battery");
	} else {
		elm_object_signal_emit(view.swipe_layout, "hide,txt,battery", "txt.battery");
		elm_object_part_text_set(view.swipe_layout, "txt.battery", "");
	}
}

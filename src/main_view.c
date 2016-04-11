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
#include "util_time.h"

#include <Elementary.h>

#define EMG_BUTTON_WIDTH 322
#define PLMN_LABEL_STYLE_START "<style=far_shadow,bottom><shadow_color=#00000033><font_size=24><align=left><color=#FFFFFF><text_class=ATO007><color_class=ATO007><wrap=none>"
#define PLMN_LABEL_STYLE_END "</wrap></color_class></text_class></color></align></font_size></shadow_color></style>"

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

static void _camera_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	View_Event_Cb cb = (View_Event_Cb)data;
	if (cb) cb();
}

void lockscreen_main_view_camera_clicked_signal_add(View_Event_Cb cb)
{
	elm_object_signal_callback_add(view.cam_layout, "camera,icon,clicked", "img.camera", _camera_clicked, cb);
}

void lockscreen_main_view_camera_clicked_signal_del()
{
	elm_object_signal_callback_del(view.cam_layout, "camera,icon,clicked", "img.camera", _camera_clicked);
}

static Evas_Event_Flags _swipe_state_end(void *data, void *event_info)
{
	_D("Swipe gesture end");
	View_Event_Cb cb = (View_Event_Cb)data;
	if (cb) cb();
	return EVAS_EVENT_FLAG_NONE;
}

Evas_Object *lockscreen_main_view_create(Evas_Object *win)
{
	view.layout = elm_layout_add(win);
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
	elm_object_part_content_set(view.layout, "sw.swipe_layout", view.swipe_layout);

	view.bg = elm_bg_add(view.layout);
	elm_bg_option_set(view.bg, ELM_BG_OPTION_SCALE);
	evas_object_size_hint_weight_set(view.bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(view.bg, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(view.layout, "sw.bg", view.bg);

	view.gesture_layer = elm_gesture_layer_add(win);
	elm_gesture_layer_hold_events_set(view.gesture_layer, EINA_TRUE);
	elm_gesture_layer_attach(view.gesture_layer, view.layout);
	elm_gesture_layer_attach(view.gesture_layer, view.swipe_layout);
	elm_gesture_layer_attach(view.gesture_layer, view.bg);
	evas_object_show(view.gesture_layer);

	return view.layout;
}

void lockscreen_main_view_unlock_signal_add(Edje_Signal_Cb cb, void *data)
{
	elm_object_signal_callback_add(view.swipe_layout, "unlock,anim,end", "swipe-layout", cb, data);
}

void lockscreen_main_view_unlock_signal_del(Edje_Signal_Cb cb)
{
	elm_object_signal_callback_del(view.swipe_layout, "unlock,anim,end", "swipe-layout", cb);
}

void lockscreen_main_view_destroy()
{
	evas_object_del(view.layout);
}

bool lockscreen_main_view_background_set(lockscreen_main_view_background_type type, const char *file)
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

void lockscreen_main_view_sim_status_text_set(const char *text)
{
	Evas_Object *label = NULL;
	char buf[512] = { 0, };
	char *markup_text = NULL;
	Evas_Object *tb = NULL;
	Evas_Coord tb_w = 0;

	label = elm_label_add(view.layout);
	ret_if(!label);

	markup_text = elm_entry_utf8_to_markup(text);
	snprintf(buf, sizeof(buf), "%s%s%s", PLMN_LABEL_STYLE_START, markup_text, PLMN_LABEL_STYLE_END);
	free(markup_text);

	elm_object_style_set(label, "slide_short");
	elm_label_wrap_width_set(label, 100);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_label_slide_duration_set(label, 2);
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);

	Evas_Object *label_edje = elm_layout_edje_get(label);
	ret_if(!label_edje);

	tb = (Evas_Object *)edje_object_part_object_get(label_edje, "elm.text");
	ret_if(!tb);

	evas_object_textblock_size_native_get(tb, &tb_w, NULL);

	if ((tb_w > 0) && (tb_w > _X(EMG_BUTTON_WIDTH))) {
		elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	}

	elm_label_slide_go(label);
	elm_object_text_set(label, buf);

	elm_object_part_content_set(view.layout, "txt.plmn", label);
	evas_object_show(label);
}

void lockscreen_main_view_swipe_signal_add(View_Event_Cb cb)
{
	elm_gesture_layer_cb_set(view.gesture_layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_END, _swipe_state_end, cb);
}

void lockscreen_main_view_swipe_signal_del(View_Event_Cb cb)
{
	elm_gesture_layer_cb_del(view.gesture_layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_END, _swipe_state_end, cb);
}

static void _layout_unlocked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	View_Event_Cb cb = (View_Event_Cb)data;
	elm_object_signal_callback_del(view.swipe_layout, "unlock,anim,end", "swipe-layout", _layout_unlocked);
	if (cb) cb();
}

void lockscreen_main_view_unlock(View_Event_Cb animation_end_cb)
{
	elm_object_signal_callback_add(view.swipe_layout, "unlock,anim,end", "swipe-layout", _layout_unlocked, animation_end_cb);
	elm_object_signal_emit(view.swipe_layout, "unlock,anim,start", "task-mgr");
	elm_object_signal_emit(view.layout, "bg,hide", "task-mgr");
}

static int _is_korea_locale(const char *locale)
{
	int ret = 0;
	if (locale) {
		if (strstr(locale,"ko_KR")) {
			ret = 1;
		}
	}
	return ret;
}

void lockscreen_main_view_time_set(const char *locale, const char *timezone, bool use24hformat, time_t time)
{
	char time_buf[PATH_MAX] = {0,};
	char date_buf[PATH_MAX] = {0,};
	char *str_date, *str_time, *str_meridiem;

	if (!util_time_formatted_time_get(time, locale, timezone, use24hformat, &str_time, &str_meridiem)) {
		_E("util_time_formatted_time_get failed");
		return;
	}
	if (!util_time_formatted_date_get(time, locale, timezone, "MMMMEd", &str_date)) {
		_E("util_time_formatted_date_get failed");
		free(str_time);
		free(str_meridiem);
		return;
	}

	if (use24hformat) {
		if (_is_korea_locale(locale)) {
			snprintf(time_buf, sizeof(time_buf), "%s", str_time);
		} else {
			snprintf(time_buf, sizeof(time_buf), "%s", str_time);
		}
	} else {
		if (_is_korea_locale(locale)) {
			snprintf(time_buf, sizeof(time_buf), "<%s>%s </>%s", "small_font", str_meridiem, str_time);
		} else {
			snprintf(time_buf, sizeof(time_buf), "%s<%s> %s</>", str_time, "small_font", str_meridiem);
		}
	}

	snprintf(date_buf, sizeof(time_buf), "<%s>%s</>", "small_font", str_date);

	elm_object_part_text_set(view.swipe_layout, "txt.time", time_buf);
	elm_object_part_text_set(view.swipe_layout, "txt.date", str_date);

	free(str_date);
	free(str_time);
	free(str_meridiem);
}

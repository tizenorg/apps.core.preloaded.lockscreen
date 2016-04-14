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

struct View_Data {
	Evas_Object *layout;
	Evas_Object *swipe_layout;
	Evas_Object *bg;
	Evas_Object *gesture_layer;
	Evas_Object *cam_layout;
};

#define VIEW_DATA_GET(obj, sd) struct View_Data *sd = evas_object_data_get(obj, "view_data");
#define VIEW_DATA_SET(obj, sd) evas_object_data_set(obj, "view_data", sd);

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

static void _camera_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	VIEW_DATA_GET(data, view);
	evas_object_callback_call(view->layout, SIGNAL_CAMERA_SELECTED, NULL);
}

bool lockscreen_main_view_camera_show(Evas_Object *obj)
{
	VIEW_DATA_GET(obj, view);
	Evas_Object *cam_ly = elm_layout_add(view->swipe_layout);
	if (!elm_layout_file_set(cam_ly, util_get_res_file_path(LOCK_EDJE_FILE), "camera-layout")) {
		FATAL("elm_layout_file_set failed");
		return false;
	}
	elm_object_part_content_set(view->swipe_layout, "sw.camera", cam_ly);
	evas_object_show(cam_ly);
	view->cam_layout = cam_ly;
	elm_object_signal_callback_add(cam_ly, "camera,icon,clicked", "camera-layout", _camera_clicked, obj);

	return true;
}

void lockscreen_main_view_camera_hide(Evas_Object *view)
{
	VIEW_DATA_GET(view, data);
	elm_object_signal_callback_del(data->cam_layout, "camera,icon,clicked", "camera-layout", _camera_clicked);
	elm_object_part_content_set(data->swipe_layout, "sw.camera", NULL);
	data->cam_layout = NULL;
}

static Evas_Event_Flags _swipe_state_end(void *data, void *event_info)
{
	struct View_Data *view = data;
	evas_object_callback_call(view->layout, SIGNAL_SWIPE_GESTURE_FINISHED, NULL);
	return EVAS_EVENT_FLAG_NONE;
}

Evas_Object *lockscreen_main_view_create(Evas_Object *win)
{
	struct View_Data *view = calloc(1, sizeof(struct View_Data));
	if (!view) return NULL;

	view->layout = elm_layout_add(win);
	if (!elm_layout_file_set(view->layout, util_get_res_file_path(LOCK_EDJE_FILE), "lockscreen")) {
		FATAL("Failed to set edje file for main view.");
		free(view);
		return NULL;
	}

	evas_object_show(view->layout);
	evas_object_size_hint_weight_set(view->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(view->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	view->swipe_layout = _swipe_layout_create(view->layout);
	if (!view->swipe_layout) {
		evas_object_del(view->layout);
		free(view);
		return NULL;
	}
	elm_object_part_content_set(view->layout, "sw.swipe_layout", view->swipe_layout);

	view->bg = elm_bg_add(view->layout);
	elm_bg_option_set(view->bg, ELM_BG_OPTION_SCALE);
	evas_object_size_hint_weight_set(view->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(view->bg, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(view->layout, "sw.bg", view->bg);

	view->gesture_layer = elm_gesture_layer_add(win);
	elm_gesture_layer_hold_events_set(view->gesture_layer, EINA_TRUE);
	elm_gesture_layer_attach(view->gesture_layer, view->layout);
	elm_gesture_layer_attach(view->gesture_layer, view->swipe_layout);
	elm_gesture_layer_attach(view->gesture_layer, view->bg);
	elm_gesture_layer_cb_set(view->gesture_layer, ELM_GESTURE_N_FLICKS, ELM_GESTURE_STATE_END, _swipe_state_end, view);
	evas_object_show(view->gesture_layer);

	VIEW_DATA_SET(view->layout, view);
	return view->layout;
}

bool lockscreen_main_view_background_set(Evas_Object *view, lockscreen_main_view_background_type type, const char *file)
{
	VIEW_DATA_GET(view, data);

	if (!elm_bg_file_set(data->bg, file, NULL)) {
		_E("elm_bg_file_set failed: %s", file);
		return false;
	}

	switch (type) {
		case LOCKSCREEN_BACKGROUND_TYPE_DEFAULT:
			elm_object_signal_emit(data->layout, "music_off", "lockscreen");
			break;
		case LOCKSCREEN_BACKGROUND_TYPE_ALBUM_ART:
			elm_object_signal_emit(data->layout, "music_on", "lockscreen");
			break;
	}

	return true;
}

void lockscreen_main_view_battery_status_text_set(Evas_Object *view, const char *battery)
{
	VIEW_DATA_GET(view, data);
	if (battery) {
		elm_object_part_text_set(data->swipe_layout, "txt.battery", battery);
		elm_object_signal_emit(data->swipe_layout, "show,txt,battery", "lockscreen");
	} else {
		elm_object_signal_emit(data->swipe_layout, "hide,txt,battery", "lockscreen");
		elm_object_part_text_set(data->swipe_layout, "txt.battery", "");
	}
}

void lockscreen_main_view_sim_status_text_set(Evas_Object *view, const char *text)
{
	VIEW_DATA_GET(view, data);
	Evas_Object *label = NULL;
	char buf[512] = { 0, };
	char *markup_text = NULL;
	Evas_Object *tb = NULL;
	Evas_Coord tb_w = 0;

	if (!text) {
		elm_object_signal_emit(data->swipe_layout, "hide,txt,plmn", "lockscreen");
		return;
	}

	label = elm_label_add(data->layout);
	markup_text = elm_entry_utf8_to_markup(text);
	snprintf(buf, sizeof(buf), "%s%s%s", PLMN_LABEL_STYLE_START, markup_text, PLMN_LABEL_STYLE_END);
	free(markup_text);

	elm_object_style_set(label, "slide_short");
	elm_label_wrap_width_set(label, 100);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_label_slide_duration_set(label, 2);
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);

	Evas_Object *label_edje = elm_layout_edje_get(label);
	tb = (Evas_Object *)edje_object_part_object_get(label_edje, "elm.text");
	if (!tb) {
		FATAL("elm.text part not found in edje");
	}

	evas_object_textblock_size_native_get(tb, &tb_w, NULL);

	if ((tb_w > 0) && (tb_w > _X(EMG_BUTTON_WIDTH))) {
		elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	}

	elm_label_slide_go(label);
	elm_object_text_set(label, buf);

	elm_object_part_content_set(data->swipe_layout, "txt.plmn", label);
	elm_object_signal_emit(data->swipe_layout, "show,txt,plmn", "lockscreen");
	evas_object_show(label);
}

static void _layout_unlocked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	VIEW_DATA_GET(data, vdata);
	evas_object_smart_callback_call(vdata->layout, SIGNAL_UNLOCK_ANIMATION_FINISHED, NULL);
	elm_object_signal_callback_del(vdata->swipe_layout, "unlock,anim,end", "swipe-layout", _layout_unlocked);
}

void lockscreen_main_view_unlock(Evas_Object *view)
{
	VIEW_DATA_GET(view, data);
	elm_object_signal_callback_add(data->swipe_layout, "unlock,anim,end", "swipe-layout", _layout_unlocked, view);
	elm_object_signal_emit(data->swipe_layout, "unlock,anim,start", "lockscreen");
	elm_object_signal_emit(data->layout, "bg,hide", "lockscreen");
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

void lockscreen_main_view_time_set(Evas_Object *view, const char *locale, const char *timezone, bool use24hformat, time_t time)
{
	VIEW_DATA_GET(view, data);
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

	elm_object_part_text_set(data->swipe_layout, "txt.time", time_buf);
	elm_object_part_text_set(data->swipe_layout, "txt.date", str_date);

	free(str_date);
	free(str_time);
	free(str_meridiem);
}

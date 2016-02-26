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

#include "window.h"
#include "log.h"


static struct {
	Evas_Object *win;
	Evas_Object *conformant;
	Evas_Object *bg;
	int win_w;
	int win_h;
} view;

Evas_Object *lock_window_win_get(void)
{
	return view.win;
}

int lock_window_width_get(void)
{
	return view.win_w;
}

int lock_window_height_get(void)
{
	return view.win_h;
}

Evas_Object *lockscreen_window_create(void)
{
	Evas_Object *win = elm_win_add(NULL, "LOCKSCREEN", ELM_WIN_NOTIFICATION);
	if (!win) return NULL;

	elm_win_alpha_set(win, EINA_TRUE);
	elm_win_title_set(win, "LOCKSCREEN");
	elm_win_borderless_set(win, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);
	elm_win_role_set(win, "notification-normal");
	elm_win_fullscreen_set(win, EINA_TRUE);
	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	elm_win_screen_size_get(win, NULL, NULL, &view.win_w, &view.win_h);

	Evas_Object *bg = elm_bg_add(win);
	elm_bg_option_set(bg, ELM_BG_OPTION_SCALE);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *conformant = elm_conformant_add(win);
	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_signal_emit(conformant, "elm,state,indicator,overlap", "elm");

	elm_win_resize_object_add(win, bg);
	elm_win_resize_object_add(win, conformant);

	evas_object_show(win);
	evas_object_show(bg);
	evas_object_show(conformant);

	view.win = win;
	view.bg = bg;
	view.conformant = conformant;

	return win;
}

void lockscreen_window_content_set(Evas_Object *content)
{
	elm_object_part_content_set(view.conformant, NULL, content);
}

bool lockscreen_window_background_image_set(const char *file)
{
	if (!elm_bg_file_set(view.bg, file, NULL)) {
		_E("elm_bg_file_set failed: %s", file);
		return false;
	}

	return true;
}

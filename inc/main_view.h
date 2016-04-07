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

#ifndef _LOCKSCREEN_MAIN_VIEW_H_
#define _LOCKSCREEN_MAIN_VIEW_H_

#include <Elementary.h>
#include <stdbool.h>

typedef enum {
	LOCKSCREEN_BACKGROUND_TYPE_DEFAULT,
	LOCKSCREEN_BACKGROUND_TYPE_ALBUM_ART,
} lockscreen_main_view_background_type;

/**
 * @brief Generic view event.
 */
typedef void (*View_Event_Cb)(void);

Evas_Object *lockscreen_main_view_create(Evas_Object *parent);

void lockscreen_main_view_destroy();

void lockscreen_main_view_camera_clicked_signal_add(Edje_Signal_Cb cb, void *data);

void lockscreen_main_view_camera_clicked_signal_del(Edje_Signal_Cb cb);

bool lockscreen_main_view_camera_show(void);

void lockscreen_main_view_camera_hide();

bool lockscreen_main_view_background_image_set(lockscreen_main_view_background_type type, const char *file);

void lockscreen_main_view_battery_status_text_set(const char *battery);

void lockscreen_main_view_music_status_set(bool enabled);

/**
 * @brief Registers callback function which will be called when
 * swipe gesture is performed on view.
 */
void lockscreen_main_view_swipe_signal_add(View_Event_Cb cb);

/**
 * @brief Deregisters callback function.
 */
void lockscreen_main_view_swipe_signal_del(View_Event_Cb cb);

bool lockscreen_main_view_background_set(lockscreen_main_view_background_type type, const char *file);

/**
 * @brief Plays unlock animation and runs @animation_end_cb on end.
 * @note animation_end_cb will be called only once.
 */
void lockscreen_main_view_unlock(View_Event_Cb animation_end_cb);

#endif

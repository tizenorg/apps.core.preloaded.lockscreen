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
 * @brief Emitted when camera icon is being selected.
 */
#define SIGNAL_CAMERA_SELECTED "camera,icon,selected"

/**
 * @brief Emitted when swipe gesture gesture has been performed.
 */
#define SIGNAL_SWIPE_GESTURE_FINISHED "swipe,gesture,finished"

/**
 * @brief Emitted unlock animation finished.
 */
#define SIGNAL_UNLOCK_ANIMATION_FINISHED "unlock,anim,finished"

/**
 * @brief Emitted unlock animation finished.
 */
#define SIGNAL_NOTIFICATION_CLICKED "notification,clicked"

Evas_Object *lockscreen_main_view_create(Evas_Object *parent);

bool lockscreen_main_view_camera_show(Evas_Object *view);

void lockscreen_main_view_camera_hide(Evas_Object *view);

bool lockscreen_main_view_background_image_set(Evas_Object *view, lockscreen_main_view_background_type type, const char *file);

void lockscreen_main_view_battery_status_text_set(Evas_Object *view, const char *battery);

bool lockscreen_main_view_background_set(Evas_Object *view, lockscreen_main_view_background_type type, const char *file);

/**
 * @brief Plays unlock animation and runs @animation_end_cb on end.
 * @note animation_end_cb will be called only once.
 */
void lockscreen_main_view_unlock(Evas_Object *obj);

void lockscreen_main_view_time_set(Evas_Object *view, const char *locale, const char *timezone, bool use24hformat, time_t time);

void lockscreen_main_view_sim_status_text_set(Evas_Object *view, const char *text);

void lockscreen_main_view_notifications_show(Evas_Object *view);

void lockscreen_main_view_notifications_hide(Evas_Object *view);

int lockscreen_main_view_notifications_append(Evas_Object *view, Evas_Object *content, void *data);

int lockscreen_main_view_notifications_append_relative(Evas_Object *view, int idx, Evas_Object *content, void *data);

Evas_Object *lockscreen_main_view_notifications_item_get(Evas_Object *view, int idx);

#endif

/*
 * Copyright 2012  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BG_H__
#define __BG_H__

#include <Elementary.h>

void _mouse_down_cb_s(void *data, Evas * evas, Evas_Object * obj, void *event_info);
void _mouse_move_cb_s(void *data, Evas * evas, Evas_Object * obj, void *event_info);
void _mouse_up_cb_s(void *data, Evas * evas, Evas_Object * obj, void *event_info);
void _slider_down_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info);
void _slider_move_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info);
void _slider_up_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info);

#endif

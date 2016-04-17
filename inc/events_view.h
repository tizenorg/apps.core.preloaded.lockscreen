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

#ifndef _LOCKSCREEN_EVENTS_VIEW_H_
#define _LOCKSCREEN_EVENTS_VIEW_H_

#define NOTI_ITEM_STYLE "noti-item"
#define NOTI_ITEM_TEXT_MAIN "elm.text"
#define NOTI_ITEM_TEXT_SUB "elm.sub.text"
#define NOTI_ITEM_TEXT_TIME "elm.time"
#define NOTI_ITEM_CONTENT_ICON "elm.icon"
#define NOTI_ITEM_CONTENT_SUB_ICON "elm.sub.icon"

#define WIDGET_ITEM_STYLE "one-icon"
#define WIDGET_ITEM_CONTENT "elm.swallow.icon"

/**
 * @brief Emitted when notification is being selected.
 */
#define SIGNAL_CAMERA_SELECTED "notification,item,selected"

#include <Elementary.h>

Evas_Object *lockscreen_events_view_create(Evas_Object *parent);

Evas_Object *lockscreen_events_genlist_get(Evas_Object *events_view);

#endif


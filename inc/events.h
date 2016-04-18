/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOCKSCREEN_EVENTS_H_
#define _LOCKSCREEN_EVENTS_H_

#include <Elementary.h>
#include <time.h>

typedef enum {
	LOCKSCREEN_EVENT_TYPE_NOTIFICATION,
	LOCKSCREEN_EVENT_TYPE_MINICONTROLLER
} lockscreen_event_type_e;

/**
 * @brief Event fired when lockscreen's events change.
 */
extern int LOCKSCREEN_EVENT_EVENTS_CHANGED;

/**
 * @brief lockscreen event handle
 */
typedef struct lockscreen_event lockscreen_event_t;

/**
 * @brief Initialize event support
 */
int lockscreen_events_init(void);

/**
 * @brief Denitialize event support.
 */
void lockscreen_events_shutdown(void);

/**
 * @brief Gets main event icon
 */
const char *lockscreen_event_icon_get(lockscreen_event_t *event);

/**
 * @brief Get secondary event icon
 */
const char *lockscreen_event_sub_icon_get(lockscreen_event_t *event);

/**
 * @brief Get event title
 */
const char *lockscreen_event_title_get(lockscreen_event_t *event);

/**
 * @brief Get event textual content.
 */
const char *lockscreen_event_content_get(lockscreen_event_t *event);

/**
 * @brief Gets time when event was posted.
 */
time_t lockscreen_event_time_get(lockscreen_event_t *event);

/**
 * @brief Launch application which posted the event
 */
bool lockscreen_event_launch(lockscreen_event_t *event);

/**
 * @brief Get event type.
 */
lockscreen_event_type_e lockscreen_event_type_get(lockscreen_event_t *event);

/**
 * @brief Gets list of all displayed events.
 *
 * @note list elements are only until LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED is fired.
 * @note should be free with eina_list_free
 */
Eina_List *lockscreen_events_get(void);

/**
 * @brief Inticates if any events for lockscreen are currently posted.
 */
bool lockscreen_events_exists(void);

/**
 * @brief Creates minicontroller for given event.
 */
Evas_Object *lockscreen_event_minicontroller_create(lockscreen_event_t *event, Evas_Object *parent);

#endif

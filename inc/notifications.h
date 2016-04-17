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

#ifndef _LOCKSCREEN_NOTIFICATIONS_H_
#define _LOCKSCREEN_NOTIFICATIONS_H_

#include <Eina.h>
#include <time.h>

/**
 * @brief Event fired when lockscreen's notification change.
 */
extern int LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED;

/**
 * @brief lockscreen notification handle
 */
typedef struct lockscreen_notification lockscreen_notification_t;

/**
 * @brief Initialize notification support
 */
int lockscreen_notifications_init(void);

/**
 * @brief Denitialize notification support.
 */
void lockscreen_notifications_shutdown(void);

/**
 * @brief Gets main notification icon
 */
const char *lockscreen_notification_icon_get(lockscreen_notification_t *event);

/**
 * @brief Get secondary notification icon
 */
const char *lockscreen_notification_sub_icon_get(lockscreen_notification_t *event);

/**
 * @brief Get notification title
 */
const char *lockscreen_notification_title_get(lockscreen_notification_t *event);

/**
 * @brief Get notification textual content.
 */
const char *lockscreen_notification_content_get(lockscreen_notification_t *event);

/**
 * @brief Gets time when notification was posted.
 */
time_t lockscreen_notification_time_get(lockscreen_notification_t *event);

/**
 * @brief Launch application which posted the notification
 */
bool lockscreen_notification_launch(lockscreen_notification_t *event);

/**
 * @brief Gets list of all displayed notifications.
 *
 * @note list elements are only until LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED is
 * fired.
 */
Eina_List *lockscreen_notifications_get(void);

/**
 * @brief Inticates if any notifications for lockscreen are currently posted.
 */
bool lockscreen_notifications_exists(void);

#endif

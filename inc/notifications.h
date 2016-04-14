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

extern int LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED;

typedef struct lockscreen_notification lockscreen_notification_t;

int lockscreen_notifications_init(void);

void lockscreen_notifications_shutdown(void);

const char *lockscreen_notification_icon_get(lockscreen_notification_t *event);

const char *lockscreen_notification_sub_icon_get(lockscreen_notification_t *event);

bool lockscreen_notification_launch(lockscreen_notification_t *event);

const Eina_List *lockscreen_notifications_get(void);

#endif

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

#ifndef _LOCKSCREEN_MINICONTROLLERS_H_
#define _LOCKSCREEN_MINICONTROLLERS_H_

#include <Elementary.h>

/**
 * @brief Event fired when minicontroller changes.
 */
extern int LOCKSCREEN_EVENT_MINICONTROLLER_CHANGED;

/**
 * @brief Initialize minicontroller support
 */
int lockscreen_minicontrollers_init(void);

/**
 * @brief Deinitialize minicontroller support.
 */
void lockscreen_minicontrollers_shutdown(void);

Evas_Object *lockscreen_minicontrollers_active_minicontroller_get(Evas_Object *parent);

bool lockscreen_minicontrollers_is_active(void);

#endif


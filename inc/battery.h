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

#ifndef _LOCKSCREEN_BATTERY_H_
#define _LOCKSCREEN_BATTERY_H_

#include <stdbool.h>

extern int LOCKSCREEN_EVENT_BATTERY_CHANGED;

int lockscreen_battery_init(void);
void lockscreen_battery_shutdown(void);
bool lockscreen_battery_is_charging(void);
bool lockscreen_battery_is_connected(void);

/**
 * @brief Returns battery level state 0 - 100
 */
int lockscreen_battery_level_get(void);

#endif

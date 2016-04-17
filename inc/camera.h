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

#ifndef _LOCKSCREEN_CAMERA_H_
#define _LOCKSCREEN_CAMERA_H_

#include <stdbool.h>

/**
 * @brief Event fired when lockscreen camera shortcut required status
 * changes. 
 *
 * @see lockscreen_camera_is_on
 */
extern int LOCKSCREEN_EVENT_CAMERA_STATUS_CHANGED;

/**
 * @brief Initializes camera module
 */
int lockscreen_camera_init(void);

/**
 * @brief Activates system-default camera application.
 */
int lockscreen_camera_activate();

/**
 * @brief Shutdowns camera module
 */
void lockscreen_camera_shutdown(void);

/**
 * @brief Returns true if camera shortcut icon should be displayed
 * on lockscreen, false otherwise.
 */
bool lockscreen_camera_is_on(void);

#endif


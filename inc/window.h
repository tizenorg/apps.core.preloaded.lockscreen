/*
 * Copyright (c) 2009-2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <stdbool.h>

Evas_Object *lock_window_win_get(void);
int lock_window_width_get(void);
int lock_window_height_get(void);

Evas_Object *lockscreen_window_create(void);
void lockscreen_window_content_set(Evas_Object *content);

#endif

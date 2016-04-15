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

#ifndef __TIME_FORMAT_CTRL_H__
#define __TIME_FORMAT_CTRL_H__

#include <Elementary.h>

int lockscreen_time_format_ctrl_init(Evas_Object *view);
void lockscreen_time_format_time_update(void);
void lockscreen_time_format_ctrl_shutdown(void);

#endif



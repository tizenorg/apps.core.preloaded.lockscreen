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

#ifndef _LOCKSCREEN_DATA_MODEL_BACKGROUND_H_
#define _LOCKSCREEN_DATA_MODEL_BACKGROUND_H_

#include "data_model.h"

int lockscreen_data_model_background_init(lockscreen_data_model_t *model);

int lockscreen_data_model_background_file_set(const char *path);

void lockscreen_data_model_background_shutdown(void);

#endif


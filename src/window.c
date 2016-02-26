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

#include <Evas.h>
#include <Ecore.h>
#include <Elementary.h>

#include "lockscreen.h"
#include "log.h"
#include "window.h"

#define STR_ATOM_PANEL_SCROLLABLE_STATE "_E_MOVE_PANEL_SCROLLABLE_STATE"

static struct _s_info {
	Evas_Object *win;
	int win_w;
	int win_h;
} s_info = {
	.win = NULL,
	.win_w = 0,
	.win_h = 0,
};

Evas_Object *lock_window_win_get(void)
{
	return s_info.win;
}

int lock_window_width_get(void)
{
	return s_info.win_w;
}

int lock_window_height_get(void)
{
	return s_info.win_h;
}

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

#include "notifications_view.h"
#include "util.h"
#include "log.h"
#include "lockscreen.h"
#include "util_time.h"

#include <Elementary.h>

Evas_Object *lockscreen_notifications_view_create(Evas_Object *parent)
{
	Evas_Object *layout = elm_layout_add(parent);
	if (!elm_layout_file_set(layout, util_get_res_file_path(LOCK_EDJE_FILE), "contextual-event")) {
		FATAL("elm_layout_file_set failed for contextual-event");
		evas_object_del(layout);
		return NULL;
	}
	evas_object_show(layout);

	Evas_Object *scroller = elm_scroller_add(layout);
	elm_scroller_content_min_limit(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	elm_object_tree_focus_allow_set(scroller, EINA_TRUE);
	evas_object_show(scroller);

	Evas_Object *box = elm_box_add(scroller);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(box);

	elm_object_content_set(scroller, box);
	elm_object_part_content_set(layout, "sw.scroller", scroller);

	return layout;
}

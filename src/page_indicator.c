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

#include <Elementary.h>

#include "lockscreen.h"
#include "log.h"
#include "page_indicator.h"
#include "contextual_event.h"

static struct _s_info {
	Evas_Object *index;
} s_info = {
	.index = NULL,
};

Evas_Object *lock_page_indicator_index_get(void)
{
	return s_info.index;
}

void lock_page_indicator_bring_in_page(int current_page)
{
	Elm_Object_Item *idx_it = NULL;

	_D("current page : %d", current_page+1);

	ret_if(!s_info.index);

	idx_it = elm_index_item_find(s_info.index, (void *)current_page);
	if (!idx_it) {
		_E("Failed to find index item");
		return;
	}

	elm_index_item_selected_set(idx_it, EINA_TRUE);
}

void lock_page_indicator_update(void)
{
	Evas_Object *contextual_event_layout = NULL;
	Evas_Object *index = NULL;

	int page_count = 0;

	contextual_event_layout = lock_contextual_event_layout_get();
	ret_if(!contextual_event_layout);

	page_count = lock_contextual_event_page_count_get();
	_D("page count : %d", page_count);

	lock_page_indicator_del();

	if (page_count > 1) {
		index = lock_page_indicator_create();
		if (index) {
			elm_object_part_content_set(contextual_event_layout, "sw.index", index);
		} else {
			_E("Failed to create page indicator");
			return;
		}

	}
}

Evas_Object *lock_page_indicator_create(void)
{
	Evas_Object *contextual_event_layout = NULL;
	Evas_Object *index = NULL;
	int page_count = 0;
	int current_page = 0;
	int i = 0;

	page_count = lock_contextual_event_page_count_get();
	_D("page count : %d", page_count);
	if (page_count == 0) {
		_E("contextual event is not exist");
		return NULL;
	}

	contextual_event_layout = lock_contextual_event_layout_get();
	retv_if(!contextual_event_layout, NULL);

	index = elm_index_add(contextual_event_layout);
	retv_if(!index, NULL);

	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(index, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_style_set(index, "pagecontrol");

	elm_index_horizontal_set(index, EINA_TRUE);
	elm_index_autohide_disabled_set(index, EINA_TRUE);
	evas_object_show(index);

	s_info.index = index;

	for (i = 0; i < page_count; i++) {
		Elm_Object_Item *idx_it = NULL;
		idx_it = elm_index_item_append(index, NULL, NULL, (void *)i);
		elm_index_item_selected_set(idx_it, EINA_TRUE);
	}

	elm_index_level_go(index, 0);

	current_page = lock_contextual_event_current_page_get();
	_D("current page : %d", current_page+1);

	lock_page_indicator_bring_in_page(current_page);

	return index;
}

void lock_page_indicator_del(void)
{
	if (s_info.index) {
		elm_index_item_clear(s_info.index);
		evas_object_del(s_info.index);
		s_info.index = NULL;
	}
}

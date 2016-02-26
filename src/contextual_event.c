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

#include <app.h>
#include <app_manager.h>
#include <app_manager_extension.h>

#include "lockscreen.h"
#include "log.h"
#include "contextual_event.h"
#include "window.h"
#include "default_lock.h"
#include "lock_time.h"
#include "battery.h"
#include "background_view.h"
#include "missed_event.h"
#include "page_indicator.h"

#define SCROLLER_PADDING 11

static struct _s_info {
	Evas_Object *contextual_event_layout;
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *music_page;
	Evas_Object *missed_event_page;
} s_info = {
	.contextual_event_layout = NULL,
	.scroller = NULL,
	.box = NULL,
	.music_page = NULL,
	.missed_event_page = NULL,
};

Evas_Object *lock_contextual_event_layout_get(void)
{
	return s_info.contextual_event_layout;
}

Evas_Object *lock_contextual_event_scroller_get(void)
{
	return s_info.scroller;
}

Evas_Object *lock_contextual_event_scroller_box_get(void)
{
	return s_info.box;
}

Eina_Bool lock_contextual_event_layout_visible_get(void)
{
	if (s_info.contextual_event_layout) {
		return EINA_TRUE;
	}

	return EINA_FALSE;
}

int lock_contextual_event_page_count_get(void)
{
	Eina_List *page_list = NULL;
	int page_count = 0;

	retv_if(!s_info.box, 0);

	page_list = elm_box_children_get(s_info.box);
	if (!page_list) {
		_E("Failed to get box page list");
		return 0;
	}

	page_count = eina_list_count(page_list);
	_D("page count : %d", page_count);

	eina_list_free(page_list);

	return page_count;
}

int lock_contextual_event_current_page_get(void)
{
	Evas_Object *scroller = NULL;
	int current_page = 0;

	scroller = lock_contextual_event_scroller_get();
	retv_if(!scroller, 0);

	elm_scroller_current_page_get(scroller, &current_page, NULL);

	return current_page;
}

static void _anim_start_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_D("start the scroller(%p) animation", scroller);
}

static void _anim_stop_cb(void *data, Evas_Object *scroller, void *event_info)
{
	_D("stop the scroller(%p) animation", scroller);

	int current_page = 0;

	elm_scroller_current_page_get(scroller, &current_page, NULL);
	_D("current page : %d", current_page+1);

	/* update page indicator */
	lock_page_indicator_bring_in_page(current_page);
}

static Evas_Object *_contextual_event_scroller_create(Evas_Object *parent)
{
	Evas_Object *box = NULL;
	Evas_Object *scroller = NULL;

	int page_width = lock_window_width_get() - (_X(SCROLLER_PADDING)*2);
	_D("page width : %d", page_width);

	retv_if(!parent, NULL);

	scroller = elm_scroller_add(parent);
	retv_if(!scroller, NULL);

	elm_scroller_content_min_limit(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_FALSE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
	elm_object_tree_focus_allow_set(scroller, EINA_TRUE);
	evas_object_show(scroller);

	elm_scroller_page_size_set(scroller, page_width, 0);
	elm_scroller_page_scroll_limit_set(scroller, 1, 1);

	evas_object_smart_callback_add(scroller, "scroll,anim,start", _anim_start_cb, NULL);
	evas_object_smart_callback_add(scroller, "scroll,anim,stop", _anim_stop_cb, NULL);

	box = elm_box_add(parent);
	if (!box) {
		_E("Failed to create box");
		evas_object_del(scroller);
		return NULL;
	}
	s_info.box = box;

	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_object_content_set(scroller, box);

	evas_object_show(box);
	elm_object_content_set(scroller, box);

	elm_scroller_page_show(scroller, 1, 0);

	return scroller;
}

static lock_error_e _terminate_app_by_name(music_state_e music_state)
{
	app_context_h app_context = NULL;
	char *name = NULL;
	int ret = 0;

	if (music_state == MUSIC_STATE_MUSIC_PLAYER_ON) {
		name = MUSIC_PLAYER;
	} else if (music_state == MUSIC_STATE_SOUND_PLAYER_ON) {
		name = SOUND_PLAYER;
	} else {
		return LOCK_ERROR_FAIL;
	}

	_D("music state : %d, name : %s", music_state, name);

	ret = app_manager_get_app_context(name, &app_context);
	if (ret != APP_MANAGER_ERROR_NONE) {
		_E("Failed to get app context");
		return LOCK_ERROR_FAIL;
	}

	ret = app_manager_terminate_app(app_context);
	if (ret != APP_MANAGER_ERROR_NONE) {
		_E("Failed to terminate app");
	}
	ret = app_context_destroy(app_context);

	return LOCK_ERROR_OK;
}

static void _close_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Eina_List *page_list = NULL;
	Evas_Object *page = NULL;
	contextual_type_e page_type = LOCK_CONTEXTUAL_TYPE_NONE;
	const char *btn_txt = NULL;
	int page_h = 0;

	ret_if(!obj);
	ret_if(!s_info.scroller);
	ret_if(!s_info.box);

	/* reset lcd off timer */
	lockscreen_lcd_off_timer_reset();

	elm_scroller_current_page_get(s_info.scroller, &page_h, NULL);
	_D("current page : %d", page_h);

	page_list = elm_box_children_get(s_info.box);
	if (!page_list) {
		_E("Failed to get box page list");
		return;
	}

	page = eina_list_nth(page_list, page_h);
	if (!page) {
		_E("Failed to get current page(%d)", page_h);
		eina_list_free(page_list);
		return;
	}
	eina_list_free(page_list);

	page_type = (contextual_type_e)evas_object_data_get(page, LOCK_CONTEXTUAL_PAGE_TYPE_KEY);
	_D("page type : %d", page_type);

	if (page_type == LOCK_CONTEXTUAL_TYPE_MISSED_EVENT) {
		btn_txt = elm_object_part_text_get(obj, "txt.close");
		if (btn_txt) {
			if (!strcmp(btn_txt, _("IDS_LCKSCN_ACBUTTON_CLOSE_ABB"))) {
				_D("close missed event : %s", btn_txt);
				lock_contextual_event_missed_event_del();
			} else if (!strcmp(btn_txt, _("IDS_ST_BUTTON_CANCEL"))) {
				elm_object_part_text_set(obj, "txt.close", _("IDS_LCKSCN_ACBUTTON_CLOSE_ABB"));
				lock_missed_event_item_selected_unset();
			} else {
				_E("btn text error : %s", btn_txt);
			}
		} else {
			_E("Failed to get btn text");
		}

		lockscreen_feedback_tap_play();
	} else if (page_type == LOCK_CONTEXTUAL_TYPE_MUSIC) {
		music_state_e music_state = lock_music_player_state_get();
		_D("close music : music state(%d)", music_state);

		lock_music_player_minicontroller_hide_event_send();
		lock_contextual_event_music_del();
		lockscreen_feedback_tap_play();

		if (LOCK_ERROR_OK != _terminate_app_by_name(music_state)) {
			_E("Failed to terminate app by name");
		}
	} else {
		_E("type error(%d)", page_type);
	}
}

Evas_Object *lock_contextual_event_layout_create(Evas_Object *parent)
{
	Evas_Object *layout = NULL;
	Evas_Object *scroller = NULL;
	Evas_Object *index = NULL;

	retv_if(!parent, NULL);

	/* create contextual event layout */
	layout = elm_layout_add(parent);
	retv_if(!layout, NULL);

	if (!elm_layout_file_set(layout, LOCK_EDJE_FILE, "contextual-event")) {
		_E("Failed to set edje file for swipe lock");
		goto ERROR;
	}

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_text_set(layout, "txt.close", _("IDS_LCKSCN_ACBUTTON_CLOSE_ABB"));
	elm_object_signal_callback_add(layout, "btn,close,clicked", "btn.close", _close_btn_clicked_cb, NULL);

	evas_object_show(layout);
	s_info.contextual_event_layout = layout;

	/* create scroller for contextual event */
	scroller = _contextual_event_scroller_create(layout);
	if (!scroller) {
		_E("Failed to create scroller");
		goto ERROR;
	}
	s_info.scroller = scroller;

	elm_object_part_content_set(layout, "sw.scroller", scroller);

	/* create page indicator for contextual event */
	index = lock_page_indicator_create();
	if (index) {
		elm_object_part_content_set(layout, "sw.index", index);
	} else {
		_E("Failed to create page indicator");
	}

	/* update time information */
	if (LOCK_ERROR_OK != lock_time_update()) {
		_E("Failed to update time & date");
	}

	/* update battery information */
	if (LOCK_ERROR_OK != lock_battery_update()) {
		_E("Failed to update battery information");
	}

	return layout;

ERROR:
	if (layout) {
		evas_object_del(layout);
		s_info.contextual_event_layout = NULL;
	}

	return NULL;
}

Evas_Object *lock_contextual_event_page_create(Evas_Object *parent, contextual_type_e type)
{
	Evas_Object *page = NULL;
	Evas_Object *bg_rect = NULL;
	int page_width = 0;
	char *group_name = NULL;

	retv_if(!parent, NULL);

	page = elm_layout_add(parent);
	retv_if(!page, NULL);

	if (type == LOCK_CONTEXTUAL_TYPE_MISSED_EVENT) {
		group_name = "missed-event-page";
	} else if (type == LOCK_CONTEXTUAL_TYPE_MUSIC) {
		group_name = "music-page";
	} else {
		_E("Failed to create contextual event page. type error(%d)", type);
		evas_object_del(page);
		return NULL;
	}

	if (!elm_layout_file_set(page, LOCK_EDJE_FILE, group_name)) {
		_E("Failed to set edje file for page");
		evas_object_del(page);
		return NULL;
	}

	evas_object_size_hint_weight_set(page, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(page, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* BG RECT */
	page_width = lock_window_width_get() - (_X(SCROLLER_PADDING)*2);
	bg_rect = evas_object_rectangle_add(evas_object_evas_get(parent));
	evas_object_color_set(bg_rect, 0, 0, 0, 0);
	evas_object_size_hint_min_set(bg_rect, page_width, 0);
	evas_object_size_hint_weight_set(bg_rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(bg_rect);
	elm_object_part_content_set(page, "bg", bg_rect);

	evas_object_data_set(page, LOCK_CONTEXTUAL_PAGE_TYPE_KEY, (void *)type);

	evas_object_show(page);

	return page;
}

Evas_Object *lock_contextual_event_page_get(contextual_type_e type)
{
	Eina_List *page_list = NULL;
	Evas_Object *page = NULL;
	Evas_Object *ret_page = NULL;
	contextual_type_e contextual_type = LOCK_CONTEXTUAL_TYPE_NONE;

	retv_if(!s_info.box, NULL);

	page_list = elm_box_children_get(s_info.box);
	retv_if(!page_list, NULL);

	EINA_LIST_FREE(page_list, page) {
		contextual_type = (contextual_type_e)evas_object_data_get(page, LOCK_CONTEXTUAL_PAGE_TYPE_KEY);
		_D("contextual type : %d", contextual_type);

		if (contextual_type == type) {
			ret_page = page;
		}
	}

	return ret_page;
}

lock_error_e lock_contextual_event_music_add(music_state_e state, const char *name, int width, int height)
{
	Evas_Object *bg = NULL;
	Evas_Object *swipe_layout = NULL;
	Evas_Object *contextual_event_layout = NULL;
	Evas_Object *music_page = NULL;
	Evas_Object *music_minicontroller = NULL;

	retv_if(!name, LOCK_ERROR_INVALID_PARAMETER);

	_D("name : %d, %s(%dx%d)", state, name, width, height);

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	contextual_event_layout = elm_object_part_content_get(swipe_layout, "sw.contextual_event");
	if (!contextual_event_layout) {
		_D("create contextual event layout");
		contextual_event_layout = lock_contextual_event_layout_create(swipe_layout);
		retv_if(!contextual_event_layout, LOCK_ERROR_FAIL);

		elm_object_part_content_set(swipe_layout, "sw.contextual_event", contextual_event_layout);
	}

	music_page = lock_contextual_event_page_get(LOCK_CONTEXTUAL_TYPE_MUSIC);
	if (!music_page) {
		music_page = lock_contextual_event_page_create(s_info.scroller, LOCK_CONTEXTUAL_TYPE_MUSIC);
		if (!music_page) {
			_E("Failed to create music page");
			if (LOCK_ERROR_OK != lock_contextual_event_layout_del()) {
				_E("Failed to delete contextual event layout");
			}
			return LOCK_ERROR_FAIL;
		}
	} else {
		_E("Failed to add music page. music page is already exist");
		return LOCK_ERROR_FAIL;
	}

	s_info.music_page = music_page;

	/* minicontroller */
	music_minicontroller = lock_music_player_minicontroller_create(state, music_page, name);
	if (music_minicontroller) {
		elm_object_part_content_set(music_page, "sw.content", music_minicontroller);
	} else {
		_E("Failed to create music minicontroller");
	}

	elm_box_pack_start(s_info.box, music_page);
	lock_page_indicator_update();

	/* background image */
	bg = lock_background_view_bg_get();
	if (bg) {
		lock_music_player_bg_set(bg, state);
	}

	return LOCK_ERROR_OK;
}

void lock_contextual_event_music_del(void)
{
	Evas_Object *page = NULL;

	lock_music_player_minicontroller_destroy();

	page = lock_contextual_event_page_get(LOCK_CONTEXTUAL_TYPE_MUSIC);
	ret_if(!page);

	evas_object_data_del(page, LOCK_CONTEXTUAL_PAGE_TYPE_KEY);
	evas_object_del(page);
	s_info.music_page = NULL;

	if (LOCK_ERROR_OK != lock_contextual_event_layout_del()) {
		_E("Failed to delete contextual event layout");
	}
}

Evas_Object *lock_contextual_event_missed_event_item_find(missed_event_type_e type)
{
	Evas_Object *swipe_layout = NULL;
	Evas_Object *contextual_event_layout = NULL;
	Evas_Object *missed_event_page = NULL;
	Evas_Object *item = NULL;
	missed_event_type_e exist_type = LOCK_MISSED_EVENT_TYPE_NONE;

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, NULL);

	contextual_event_layout = elm_object_part_content_get(swipe_layout, "sw.contextual_event");
	retv_if(!contextual_event_layout, NULL);

	missed_event_page = lock_contextual_event_page_get(LOCK_CONTEXTUAL_TYPE_MISSED_EVENT);
	retv_if(!missed_event_page, NULL);

	item = elm_object_part_content_get(missed_event_page, "sw.content.top");
	exist_type = (missed_event_type_e)evas_object_data_get(item, LOCK_CONTEXTUAL_MISSED_EVENT_TYPE_KEY);
	if (exist_type == type) {
		_D("item(type : %d) is exist in top position", type);
		return item;
	}

	item = elm_object_part_content_get(missed_event_page, "sw.content.bottom");
	exist_type = (missed_event_type_e)evas_object_data_get(item, LOCK_CONTEXTUAL_MISSED_EVENT_TYPE_KEY);
	if (exist_type == type) {
		_D("item(type : %d) is exist in bottom position", type);
		return item;
	}

	return NULL;
}

lock_error_e lock_contextual_event_missed_event_item_add(Evas_Object *item, missed_event_type_e type)
{
	Evas_Object *swipe_layout = NULL;
	Evas_Object *contextual_event_layout = NULL;
	Evas_Object *missed_event_page = NULL;
	Evas_Object *item_top = NULL;

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	retv_if(!item, LOCK_ERROR_FAIL);

	contextual_event_layout = elm_object_part_content_get(swipe_layout, "sw.contextual_event");
	if (!contextual_event_layout) {
		_D("create contextual event layout");
		contextual_event_layout = lock_contextual_event_layout_create(swipe_layout);
		retv_if(!contextual_event_layout, LOCK_ERROR_FAIL);

		elm_object_part_content_set(swipe_layout, "sw.contextual_event", contextual_event_layout);
	}

	missed_event_page = lock_contextual_event_page_get(LOCK_CONTEXTUAL_TYPE_MISSED_EVENT);
	if (!missed_event_page) {
		missed_event_page = lock_contextual_event_page_create(s_info.scroller, LOCK_CONTEXTUAL_TYPE_MISSED_EVENT);
		if (!missed_event_page) {
			_E("Failed to create missed event page");
			if (LOCK_ERROR_OK != lock_contextual_event_layout_del()) {
				_E("Failed to delete contextual event layout");
			}
			return LOCK_ERROR_FAIL;
		}
		elm_box_pack_start(s_info.box, missed_event_page);
		lock_page_indicator_update();
	}

	s_info.missed_event_page = missed_event_page;

	item_top = elm_object_part_content_get(missed_event_page, "sw.content.top");
	if (item_top) {
		_D("top position is not empty");
		elm_object_part_content_set(missed_event_page, "sw.content.bottom", item);
	} else {
		_D("top position is empty");
		elm_object_part_content_set(missed_event_page, "sw.content.top", item);
	}

	return LOCK_ERROR_OK;
}

void lock_contextual_event_missed_event_item_del(missed_event_type_e del_type)
{
	Evas_Object *page = NULL;
	Evas_Object *item = NULL;
	missed_event_type_e type = LOCK_MISSED_EVENT_TYPE_NONE;

	page = lock_contextual_event_page_get(LOCK_CONTEXTUAL_TYPE_MISSED_EVENT);
	ret_if(!page);

	/* check top item */
	item = elm_object_part_content_get(page, "sw.content.top");
	if (!item) {
		_E("Failed to get top item");

		evas_object_data_del(page, LOCK_CONTEXTUAL_PAGE_TYPE_KEY);
		evas_object_del(page);
		s_info.missed_event_page = NULL;

		if (LOCK_ERROR_OK != lock_contextual_event_layout_del()) {
			_E("Failed to delete contextual event layout");
		}

		return;
	}

	type = (missed_event_type_e)evas_object_data_get(item, LOCK_CONTEXTUAL_MISSED_EVENT_TYPE_KEY);
	_D("top item type : %d", type);

	if (type == del_type) {
		_D("delete item in top position(%d)", type);
		lock_missed_event_item_destroy(item);
		item = NULL;

		item = elm_object_part_content_unset(page, "sw.content.bottom");
		if (item) {
			_D("move a item from bottom to top");
			elm_object_part_content_set(page, "sw.content.top", item);
		} else {
			_D("missed event items are not exist");
			evas_object_data_del(page, LOCK_CONTEXTUAL_PAGE_TYPE_KEY);
			evas_object_del(page);
			s_info.missed_event_page = NULL;

			if (LOCK_ERROR_OK != lock_contextual_event_layout_del()) {
				_E("Failed to delete contextual event layout");
			}
		}

		return;
	}

	/* check bottom item */
	item = elm_object_part_content_get(page, "sw.content.bottom");
	if (!item) {
		_E("Failed to get bottom item");
		return;
	}

	type = (missed_event_type_e)evas_object_data_get(item, LOCK_CONTEXTUAL_MISSED_EVENT_TYPE_KEY);
	_D("bottom item type : %d", type);

	if (type == del_type) {
		_D("delete item in bottom position(%d)", type);
		lock_missed_event_item_destroy(item);
		item = NULL;
	}

	return;
}

void lock_contextual_event_missed_event_del(void)
{
	Evas_Object *page = NULL;
	Evas_Object *item = NULL;

	page = lock_contextual_event_page_get(LOCK_CONTEXTUAL_TYPE_MISSED_EVENT);
	ret_if(!page);

	item = elm_object_part_content_get(page, "sw.content.top");
	if (item) {
		_D("delete item in top position");
		lock_missed_event_item_destroy(item);
		item = NULL;
	}

	item = elm_object_part_content_get(page, "sw.content.bottom");
	if (item) {
		_D("delete item in bottom position");
		lock_missed_event_item_destroy(item);
		item = NULL;
	}

	evas_object_data_del(page, LOCK_CONTEXTUAL_PAGE_TYPE_KEY);
	evas_object_del(page);
	s_info.missed_event_page = NULL;

	if (LOCK_ERROR_OK != lock_contextual_event_layout_del()) {
		_E("Failed to delete contextual event layout");
	}
}

lock_error_e lock_contextual_event_layout_del(void)
{
	int page_count = 0;

	retv_if(!s_info.contextual_event_layout, LOCK_ERROR_FAIL);
	retv_if(!s_info.box, LOCK_ERROR_FAIL);

	page_count = lock_contextual_event_page_count_get();
	if (page_count != 0) {
		_E("contextual event is exist");

		/* update page indicator */
		lock_page_indicator_update();

		return LOCK_ERROR_OK;
	}

	if (s_info.box) {
		evas_object_del(s_info.box);
		s_info.box = NULL;
	}

	if (s_info.scroller) {
		evas_object_del(s_info.scroller);
		s_info.scroller = NULL;
	}

	lock_page_indicator_del();

	if (s_info.contextual_event_layout) {
		evas_object_del(s_info.contextual_event_layout);
		s_info.contextual_event_layout = NULL;
	}

	if (LOCK_ERROR_OK != lock_time_update()) {
		_E("Failed to update time & date");
	}

	if (LOCK_ERROR_OK != lock_battery_update()) {
		_E("Failed to update battery information");
	}

	return LOCK_ERROR_OK;
}

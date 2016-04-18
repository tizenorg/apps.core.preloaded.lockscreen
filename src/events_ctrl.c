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

#include "log.h"
#include "events_ctrl.h"
#include "events_view.h"
#include "main_view.h"
#include "notifications.h"
#include "minicontrollers.h"

#include <Ecore.h>
#include <time.h>

static Ecore_Event_Handler *events_handler, *mini_handler;
static Evas_Object *main_view;
static Elm_Object_Item *active_minicontroller;

static Evas_Object *_lockscreen_events_view_ctrl_genlist_noti_content_get(void *data, Evas_Object *obj, const char *part);
static char *_lockscreen_events_view_ctrl_genlist_noti_text_get(void *data, Evas_Object *obj, const char *part);
static Evas_Object *_lockscreen_events_view_ctrl_genlist_widget_content_get(void *data, Evas_Object *obj, const char *part);

static Elm_Genlist_Item_Class noti_itc = {
	.item_style = "double_label",
	.func.content_get = _lockscreen_events_view_ctrl_genlist_noti_content_get,
	.func.text_get = _lockscreen_events_view_ctrl_genlist_noti_text_get,
};

static Elm_Genlist_Item_Class widget_itc = {
	.item_style = WIDGET_ITEM_STYLE,
	.func.content_get = _lockscreen_events_view_ctrl_genlist_widget_content_get,
};

static Evas_Object *_lockscreen_events_view_ctrl_genlist_noti_content_get(void *data, Evas_Object *obj, const char *part)
{
	return NULL;
}

static char *_lockscreen_events_view_ctrl_genlist_noti_text_get(void *data, Evas_Object *obj, const char *part)
{
	lockscreen_notification_t *noti = data;
	const char *val = NULL;

	if (!strcmp(part, "elm.text")) {
		val = lockscreen_notification_title_get(noti);
	}
	else if (!strcmp(part, "elm.text.end")) {
		val = lockscreen_notification_content_get(noti);
	}
	return val ? strdup(val) : NULL;
}

static Evas_Object *_lockscreen_events_view_ctrl_genlist_widget_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, WIDGET_ITEM_CONTENT)) {
		return lockscreen_minicontrollers_active_minicontroller_get(obj);
	}
	return NULL;
}

static void _lockscreen_events_ctrl_view_show()
{
	Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
	if (!events_view) {
		events_view = lockscreen_events_view_create(main_view);
		lockscreen_main_view_part_content_set(main_view, PART_EVENTS, events_view);
	}
}

static void _lockscreen_events_ctrl_view_hide()
{
	Evas_Object *events_view = lockscreen_main_view_part_content_get(main_view, PART_EVENTS);
	if (events_view) {
		evas_object_del(events_view);
	}
}

static int _lockscreen_events_ctrl_sort(const void *data1, const void *data2)
{
	return EINA_TRUE;
}

static void _lockscreen_events_ctrl_notifications_load()
{
	lockscreen_notification_t *lnoti;

	Evas_Object *genlist = lockscreen_events_genlist_get(lockscreen_main_view_part_content_get(main_view, PART_EVENTS));
	if (!genlist) {
		FATAL("lockscreen_events_genlist_get failed");
		return;
	}

	Eina_List *notis = lockscreen_notifications_get();
	notis = eina_list_sort(notis, -1, _lockscreen_events_ctrl_sort);
	EINA_LIST_FREE(notis, lnoti) {
		Elm_Genlist_Item *it = elm_genlist_item_append(genlist, &noti_itc, lnoti, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		if (!it) FATAL("elm_genlist_item_append failed!");
	}
}

static void _lockscreen_events_ctrl_notifications_unload()
{
	Evas_Object *genlist = lockscreen_events_genlist_get(lockscreen_main_view_part_content_get(main_view, PART_EVENTS));
	Elm_Genlist_Item *it;
	Eina_List *items = elm_genlist_realized_items_get(genlist);
	EINA_LIST_FREE(items, it) {
		if (it != active_minicontroller)
			elm_object_item_del(it);
	}
}

static void _lockscreen_events_ctrl_minicontroller_load()
{
	Evas_Object *genlist = lockscreen_events_genlist_get(lockscreen_main_view_part_content_get(main_view, PART_EVENTS));
	if (!genlist) {
		FATAL("lockscreen_events_genlist_get failed");
		return;
	}

	active_minicontroller = elm_genlist_item_prepend(genlist, &widget_itc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

static void _lockscreen_events_ctrl_minicontroller_unload()
{
	if (active_minicontroller)
		elm_object_item_del(active_minicontroller);
}

static Eina_Bool _lockscreen_events_ctrl_notifications_changed(void *data, int event, void *event_info)
{
	_lockscreen_events_ctrl_notifications_unload();

	if (lockscreen_notifications_exists()) {
		_lockscreen_events_ctrl_view_show();
		_lockscreen_events_ctrl_notifications_load();
	}
	else {
		_lockscreen_events_ctrl_view_hide();
	}

	return EINA_TRUE;
}

static Eina_Bool _lockscreen_events_ctrl_minicontroller_changed(void *data, int event, void *event_info)
{
	_lockscreen_events_ctrl_minicontroller_unload();

	if (lockscreen_minicontrollers_is_active()) {
		_lockscreen_events_ctrl_view_show();
		_lockscreen_events_ctrl_minicontroller_load();
	}
	else
		_lockscreen_events_ctrl_view_hide();

	return EINA_TRUE;
}

int lockscreen_events_ctrl_init(Evas_Object *mv)
{
	if (lockscreen_notifications_init()) {
		FATAL("lockscreen_notifications_init failed.");
		return 1;
	}

	if (lockscreen_minicontrollers_init()) {
		FATAL("lockscreen_minicontrollers_init failed.");
		lockscreen_notifications_shutdown();
		return 1;
	}

	main_view = mv;

	events_handler = ecore_event_handler_add(LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED, _lockscreen_events_ctrl_notifications_changed, NULL);
	mini_handler = ecore_event_handler_add(LOCKSCREEN_EVENT_MINICONTROLLER_CHANGED, _lockscreen_events_ctrl_minicontroller_changed, NULL);

	if (lockscreen_notifications_exists()) {
		_lockscreen_events_ctrl_view_show();
	}
	else
		_lockscreen_events_ctrl_view_hide();

	return 0;
}

void lockscreen_events_ctrl_shutdown()
{
	ecore_event_handler_del(mini_handler);
	ecore_event_handler_del(events_handler);
	lockscreen_notifications_shutdown();
	lockscreen_minicontrollers_shutdown();
}

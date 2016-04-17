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

#include <Ecore.h>
#include <time.h>

static Ecore_Event_Handler *handler;
static Evas_Object *main_view;
static Elm_Object_Item *active_minicontroller;

static Elm_Genlist_Item_Class itc = {
};

static void _lockscreen_events_ctrl_view_show()
{
}

static void _lockscreen_events_ctrl_view_hide()
{
}

static Eina_Bool _lockscreen_events_ctrl_sort(const void *data, const void *data)
{
	return EINA_TRUE;
}

static void _lockscreen_events_ctrl_notifications_load(Evas_Object *genlist)
{
	lockscreen_notification_t *lnoti;

	Eina_List *notis = lockscreen_notifications_get();
	notis = eina_list_sort(notis, -1, _lockscreen_events_ctrl_sort, NULL);
	EINA_LIST_FREE(notis, lnoti) {
		elm_genlist_item_append(genlist, &itc, NULL, ELM_GENLIST_ITEM_NONE, lnoti, NULL. NULL);
	}
}

static void _lockscreen_events_ctrl_notifications_unload(Evas_Object *genlist)
{
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

int lockscreen_events_ctrl_init(Evas_Object *mv)
{
	if (lockscreen_notifications_init()) {
		FATAL("lockscreen_notifications_init failed.");
		return 1;
	}

	handler = ecore_event_handler_add(LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED, _lockscreen_events_ctrl_notifications_changed, NULL);
	main_view = mv;

	if (lockscreen_notifications_exists())
		_lockscreen_events_ctrl_view_show();
	else
		_lockscreen_events_ctrl_view_hide();

	return 0;
}

void lockscreen_events_ctrl_shutdown()
{
	ecore_event_handler_del(handler);
	lockscreen_notifications_shutdown();
}

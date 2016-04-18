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

#include <notification.h>
#include <notification_list.h>
#include <notification_internal.h>
#include <app_control_internal.h>
#include <Ecore.h>

#include "log.h"
#include "events.h"
#include "minicontrollers.h"

static Eina_List *events;
static int init_count;
int LOCKSCREEN_EVENT_EVENTS_CHANGED;

struct lockscreen_event {
	lockscreen_event_type_e type;
	union {
		char *icon_path;
		char *minicontroller_name;
	};
	char *icon_sub_path;
	char *title;
	char *content;
	bundle *service_handle;
	time_t time;
	char *package;
};

static bool _notification_accept(notification_h noti)
{
	int app_list = 0;
	int ret = notification_get_display_applist(noti, &app_list);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_display_applist failed: %s", get_error_message(ret));
		return false;
	}
	return app_list & NOTIFICATION_DISPLAY_APP_LOCK;
}

static void _lockscreen_event_destroy(lockscreen_event_t *event)
{
	if (event->title) free(event->title);
	if (event->content) free(event->content);
	if (event->icon_path) free(event->icon_path);
	if (event->icon_sub_path) free(event->icon_sub_path);
	if (event->package) free(event->package);
	if (event->service_handle) bundle_free(event->service_handle);

	free(event);
}

static lockscreen_event_t *_lockscreen_event_create(notification_h noti)
{
	int ret;
	char *val;
	lockscreen_event_t *event = calloc(1, sizeof(lockscreen_event_t));
	if (!event) return NULL;

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_text failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->title = strdup(val);

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_text failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->content = strdup(val);

	ret = notification_get_pkgname(noti, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_pkgname failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->package = strdup(val);

	ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_image failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->icon_path = strdup(val);

	ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_SUB, &val);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_image failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}
	if (val) event->icon_sub_path = strdup(val);

	ret = notification_get_time(noti, &event->time);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_time failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}

	ret = notification_get_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, &event->service_handle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_execute_option failed: %s", get_error_message(ret));
		_lockscreen_event_destroy(event);
		return NULL;
	}

	if (event->service_handle) {
		event->service_handle = bundle_dup(event->service_handle);
	}

	_D("Title: %s", event->title);
	_D("Content: %s", event->content);
	_D("Package: %s", event->package);
	_D("Icon: %s", event->icon_path);
	_D("SubIcon: %s", event->icon_sub_path);

	return event;
}

static int load_notifications()
{
	notification_list_h noti_list;
	notification_list_h noti_list_head = NULL;
	notification_h noti = NULL;

	int ret = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &noti_list_head);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_list failed: %s", get_error_message(ret));
		return 1;
	}

	noti_list = noti_list_head;
	while (noti_list) {
		noti = notification_list_get_data(noti_list);
		if (_notification_accept(noti)) {
			lockscreen_event_t *me = _lockscreen_event_create(noti);
			events = eina_list_append(events, me);
		}
		noti_list = notification_list_get_next(noti_list);
	}

	ecore_event_add(LOCKSCREEN_EVENT_EVENTS_CHANGED, NULL, NULL, NULL);

	notification_free_list(noti_list_head);
	return 0;
}

static void unload_notifications()
{
	lockscreen_event_t *event;

	if (!events)
		return;

	EINA_LIST_FREE(events, event)
		_lockscreen_event_destroy(event);

	events = NULL;
	ecore_event_add(LOCKSCREEN_EVENT_EVENTS_CHANGED, NULL, NULL, NULL);
}

static void _noti_changed_cb(void *data, notification_type_e type, notification_op *op_list, int num_op)
{
	unload_notifications();
	load_notifications();
}

int lockscreen_events_init(void)
{
	if (!init_count) {
		LOCKSCREEN_EVENT_EVENTS_CHANGED = ecore_event_type_new();
		int ret = notification_register_detailed_changed_cb(_noti_changed_cb, NULL);
		if (ret != NOTIFICATION_ERROR_NONE) {
			_E("event_register_detailed_changed_cb failed: %d", get_error_message(ret));
			return 1;
		}
		if (load_notifications()) {
			_E("load_notifications failed");
			return 1;
		}
	}
	init_count++;
	return 0;
}

void lockscreen_events_shutdown(void)
{
	if (init_count) {
		init_count--;
		if (!init_count) {
			int ret = notification_unregister_detailed_changed_cb(_noti_changed_cb, NULL);
			if (ret != NOTIFICATION_ERROR_NONE) {
				_E("event_unregister_detailed_changed_cb failed: %s", get_error_message(ret));
			}
			unload_notifications();
		}
	}
}

bool lockscreen_event_launch(lockscreen_event_t *event)
{
	app_control_h service = NULL;

	int ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		_E("app_control_create failed: %s", get_error_message(ret));
		return false;
	}

	ret = app_control_import_from_bundle(service, event->service_handle);
	if (ret != APP_CONTROL_ERROR_NONE) {
		_E("app_control_import_from_bundle: %s", get_error_message(ret));
		app_control_destroy(service);
		return false;
	}

	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		_E("app_control_send_launch_request failed: %s", get_error_message(ret));
		app_control_destroy(service);
		return false;
	}

	app_control_destroy(service);

	return true;
}

Eina_List *lockscreen_events_get(void)
{
	return eina_list_clone(events);
}

bool lockscreen_events_exists(void)
{
	return events ? EINA_TRUE : EINA_FALSE;
}

const char *lockscreen_event_title_get(lockscreen_event_t *event)
{
	return event->title;
}

const char *lockscreen_event_content_get(lockscreen_event_t *event)
{
	return event->content;
}

time_t lockscreen_event_time_get(lockscreen_event_t *event)
{
	return event->time;
}

const char *lockscreen_event_icon_get(lockscreen_event_t *event)
{
	return event->icon_path;
}

const char *lockscreen_event_sub_icon_get(lockscreen_event_t *event)
{
	return event->icon_sub_path;
}

lockscreen_event_type_e lockscreen_event_type_get(lockscreen_event_t *event)
{
	return event->type;

}

Evas_Object *lockscreen_event_minicontroller_create(lockscreen_event_t *event, Evas_Object *parent)
{
	if (event->type != LOCKSCREEN_EVENT_TYPE_MINICONTROLLER)
		return NULL;
	return lockscreen_minicontrollers_minicontroller_create(event->minicontroller_name, parent);
}

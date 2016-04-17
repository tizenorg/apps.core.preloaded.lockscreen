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
#include "notifications.h"

static Eina_List *notifications;
static int init_count;
int LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED;

struct lockscreen_notification {
	char *icon_path;
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

static void _lockscreen_notification_destroy(lockscreen_notification_t *event)
{
	if (event->title) free(event->title);
	if (event->content) free(event->content);
	if (event->icon_path) free(event->icon_path);
	if (event->icon_sub_path) free(event->icon_sub_path);
	if (event->package) free(event->package);
	if (event->service_handle) bundle_free(event->service_handle);

	free(event);
}

static lockscreen_notification_t *_lockscreen_notification_create(notification_h noti)
{
	int ret;
	lockscreen_notification_t *event = calloc(1, sizeof(lockscreen_notification_t));
	if (!event) return NULL;

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &event->title);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_text failed: %s", get_error_message(ret));
		_lockscreen_notification_destroy(event);
		return NULL;
	}

	ret = notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &event->content);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_text failed: %s", get_error_message(ret));
		_lockscreen_notification_destroy(event);
		return NULL;
	}

	ret = notification_get_pkgname(noti, &event->package);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_pkgname failed: %s", get_error_message(ret));
		_lockscreen_notification_destroy(event);
		return NULL;
	}

	ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &event->icon_path);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_image failed: %s", get_error_message(ret));
		_lockscreen_notification_destroy(event);
		return NULL;
	}

	ret = notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_SUB, &event->icon_sub_path);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_image failed: %s", get_error_message(ret));
		_lockscreen_notification_destroy(event);
		return NULL;
	}

	ret = notification_get_time(noti, &event->time);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_time failed: %s", get_error_message(ret));
		_lockscreen_notification_destroy(event);
		return NULL;
	}

	ret = notification_get_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, &event->service_handle);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("notification_get_execute_option failed: %s", get_error_message(ret));
		_lockscreen_notification_destroy(event);
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
			lockscreen_notification_t *me = _lockscreen_notification_create(noti);
			notifications = eina_list_append(notifications, me);
		}
		noti_list = notification_list_get_next(noti_list);
	}

	ecore_event_add(LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED, NULL, NULL, NULL);

	notification_free_list(noti_list_head);
	return 0;
}

static void unload_notifications()
{
	lockscreen_notification_t *event;

	if (!notifications)
		return;

	EINA_LIST_FREE(notifications, event)
		_lockscreen_notification_destroy(event);

	notifications = NULL;
	ecore_event_add(LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED, NULL, NULL, NULL);
}

static void _noti_changed_cb(void *data, notification_type_e type, notification_op *op_list, int num_op)
{
	unload_notifications();
	load_notifications();
}

int lockscreen_notifications_init(void)
{
	if (!init_count) {
		LOCKSCREEN_EVENT_NOTIFICATIONS_CHANGED = ecore_event_type_new();
		int ret = notification_register_detailed_changed_cb(_noti_changed_cb, NULL);
		if (ret != NOTIFICATION_ERROR_NONE) {
			_E("notification_register_detailed_changed_cb failed: %d", get_error_message(ret));
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

void lockscreen_notifications_shutdown(void)
{
	if (init_count) {
		init_count--;
		if (!init_count) {
			int ret = notification_unregister_detailed_changed_cb(_noti_changed_cb, NULL);
			if (ret != NOTIFICATION_ERROR_NONE) {
				_E("notification_unregister_detailed_changed_cb failed: %s", get_error_message(ret));
			}
			unload_notifications();
		}
	}
}

bool lockscreen_notification_launch(lockscreen_notification_t *event)
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

Eina_List *lockscreen_notifications_get(void)
{
	return eina_list_clone(notifications);
}

Eina_Bool lockscreen_notifications_exists(void)
{
	return notifications ? EINA_TRUE : EINA_FALSE;
}

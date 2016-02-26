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

#include <notification.h>
#include <notification_internal.h>
#include <notification_list.h>
#include <bundle_internal.h>
#include <app_control_internal.h>

#include "lockscreen.h"
#include "log.h"
#include "missed_event.h"
#include "default_lock.h"
#include "contextual_event.h"
#include "lock_time.h"
#include "property.h"

static const int MAX_CONTENT = 128;

static struct _s_info {
	Evas_Object *selected_item;
} s_info = {
	.selected_item = NULL,
};

Evas_Object *lock_missed_event_selected_item_get(void)
{
	return s_info.selected_item;
}


static void _missed_event_item_icon_destroy(Evas_Object *icon)
{
	ret_if(!icon);

	evas_object_del(icon);
}

static void _missed_event_item_icon_update(Evas_Object *item, const char *icon_path, const char *icon_sub_path, missed_event_type_e event_type)
{
	Evas_Object *icon_layout = NULL;
	Evas_Object *icon = NULL;
	Evas_Object *icon_sub = NULL;

	ret_if(!item);
	ret_if(!icon_path);
	ret_if(!icon_sub_path);

	icon_layout = elm_object_part_content_get(item, "sw.icon");
	ret_if(!icon_layout);

	/* icon */
	icon = elm_object_part_content_get(icon_layout, "sw.icon");
	ret_if(!icon);

	if (!elm_image_file_set(icon, icon_path, NULL)) {
		char *default_icon_path = NULL;

		_E("Failed to set a missed event icon");

		if (event_type == LOCK_MISSED_EVENT_TYPE_CALL) {
			default_icon_path = ICON_PATH_MISSED_EVENT_CALL;
		} else if (event_type == LOCK_MISSED_EVENT_TYPE_MSG) {
			default_icon_path = ICON_PATH_MISSED_EVENT_MSG;
		}

		if (!elm_image_file_set(icon, default_icon_path, NULL)) {
			_E("Failed to set defauilt missed event icon");
		}
	}

	/* sub icon */
	icon_sub = elm_object_part_content_get(icon_layout, "sw.icon.sub");
	ret_if(!icon_sub);

	if (!elm_image_file_set(icon_sub, icon_sub_path, NULL)) {
		_E("Failed to set a missed sub icon");
	}
}

static Evas_Object *_missed_event_item_icon_create(Evas_Object *parent, const char *icon_path, const char *icon_sub_path, missed_event_type_e event_type)
{
	Evas_Object *icon_layout = NULL;
	Evas_Object *icon = NULL;
	Evas_Object *icon_sub = NULL;

	retv_if(!parent, NULL);

	icon_layout = elm_layout_add(parent);
	retv_if(!icon_layout, NULL);

	if (!elm_layout_file_set(icon_layout, LOCK_EDJE_FILE, "missed-event-item-icon")) {
		_E("Failed to set edje file for missed event item icon");
		return NULL;
	}

	icon = elm_image_add(icon_layout);
	goto_if(!icon, error);

	if (!elm_image_file_set(icon, icon_path, NULL)) {
		char *default_icon_path = NULL;

		_E("Failed to set missed event icon");

		if (event_type == LOCK_MISSED_EVENT_TYPE_CALL) {
			default_icon_path = ICON_PATH_MISSED_EVENT_CALL;
		} else if (event_type == LOCK_MISSED_EVENT_TYPE_MSG) {
			default_icon_path = ICON_PATH_MISSED_EVENT_MSG;
		}

		if (!elm_image_file_set(icon, default_icon_path, NULL)) {
			_E("Failed to set defauilt missed event icon");
		}
	}

	elm_image_aspect_fixed_set(icon, EINA_TRUE);
	elm_image_fill_outside_set(icon, EINA_TRUE);

	elm_object_part_content_set(icon_layout, "sw.icon", icon);

	icon_sub = elm_image_add(icon_layout);
	if (icon_sub) {
		if (!elm_image_file_set(icon_sub, icon_sub_path, NULL)) {
			_E("Failed to set missed event sub icon");
			evas_object_del(icon_sub);
		} else {
			elm_object_part_content_set(icon_layout, "sw.icon.sub", icon_sub);
		}
	}

	evas_object_show(icon);
	evas_object_show(icon_sub);
	evas_object_show(icon_layout);

	return icon_layout;

error:
	if (icon_layout) {
		evas_object_del(icon_layout);
	}

	return NULL;
}

void lock_missed_event_item_launch(void)
{
	app_control_h service = NULL;
	bundle *single_service_handle = NULL;
	int ret = APP_CONTROL_ERROR_NONE;

	ret_if(!s_info.selected_item);

	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		_E("Failed to create app control(%d)", ret);
		return;
	}

	ret_if(!service);

	single_service_handle = (bundle *)evas_object_data_get(s_info.selected_item, LOCK_CONTEXTUAL_MISSED_EVENT_BUNDLE_KEY);
	if (single_service_handle) {
		ret = app_control_import_from_bundle(service, single_service_handle);
		if (ret != APP_CONTROL_ERROR_NONE) {
			_E("Failed to import app control from bundle(%d)", ret);
			app_control_destroy(service);
			return;
		}
	}

	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		_E("Failed to send launch request(%d)", ret);
	}

	app_control_destroy(service);

	return;
}

void lock_missed_event_item_disabled_set(Eina_Bool disabled)
{
	Evas_Object *page = NULL;
	Evas_Object *item_top = NULL;
	Evas_Object *item_bottom = NULL;
	Evas_Object *item_icon = NULL;

	ret_if(!s_info.selected_item);

	page = lock_contextual_event_page_get(LOCK_CONTEXTUAL_TYPE_MISSED_EVENT);
	ret_if(!page);

	item_top = elm_object_part_content_get(page, "sw.content.top");
	item_bottom = elm_object_part_content_get(page, "sw.content.bottom");

	if (item_top == s_info.selected_item) {
		if (item_bottom) {
			item_icon = elm_object_part_content_get(item_bottom, "sw.icon");

			if (disabled) {
				_D("bottom item is disabled");
				elm_object_signal_emit(item_bottom, "set,disabled", "bg");
				if (item_icon) {
					evas_object_color_set(item_icon, 0, 0, 0, 50);
				}
			} else {
				_D("bottom item is enabled");
				elm_object_signal_emit(item_bottom, "set,default", "bg");
				if (item_icon) {
					evas_object_color_set(item_icon, 255, 255, 255, 255);
				}
			}
		}
	} else if (item_bottom == s_info.selected_item) {
		item_icon = elm_object_part_content_get(item_top, "sw.icon");

		if (disabled) {
			_D("top item is disabled");
			elm_object_signal_emit(item_top, "set,disabled", "bg");
			if (item_icon) {
				evas_object_color_set(item_icon, 0, 0, 0, 50);
			}
		} else {
			_D("top item is enabled");
			elm_object_signal_emit(item_top, "set,default", "bg");
			if (item_icon) {
				evas_object_color_set(item_icon, 255, 255, 255, 255);
			}
		}
	}
}

void lock_missed_event_item_selected_unset(void)
{
	Evas_Object *contextual_event_scroller = NULL;
	Evas_Object *contextual_event_layout = NULL;
	bool is_selected = false;

	contextual_event_scroller = lock_contextual_event_scroller_get();
	ret_if(!contextual_event_scroller);

	contextual_event_layout = lock_contextual_event_layout_get();
	ret_if(!contextual_event_layout);

	ret_if(!s_info.selected_item);

	is_selected = (bool)evas_object_data_get(s_info.selected_item, LOCK_CONTEXTUAL_MISSED_EVENT_IS_SELECTED_KEY);
	_D("selected : %d", is_selected);

	if (is_selected) {
		lock_missed_event_item_disabled_set(EINA_FALSE);

		/* unset blocking of scrolling */
		elm_scroller_movement_block_set(contextual_event_scroller, ELM_SCROLLER_MOVEMENT_NO_BLOCK);

		elm_object_part_text_set(contextual_event_layout, "txt.close", _("IDS_LCKSCN_ACBUTTON_CLOSE_ABB"));
		elm_object_signal_emit(s_info.selected_item, "set,default", "bg");
		evas_object_data_set(s_info.selected_item, LOCK_CONTEXTUAL_MISSED_EVENT_IS_SELECTED_KEY, false);
		s_info.selected_item = NULL;
	}
}

static void _missed_event_item_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *contextual_event_scroller = NULL;
	Evas_Object *contextual_event_layout = NULL;
	Evas_Object *item = (Evas_Object *)data;
	bool is_selected = false;
	missed_event_type_e type = LOCK_MISSED_EVENT_TYPE_NONE;

	contextual_event_scroller = lock_contextual_event_scroller_get();
	ret_if(!contextual_event_scroller);

	contextual_event_layout = lock_contextual_event_layout_get();
	ret_if(!contextual_event_layout);

	ret_if(!item);

	ret_if(s_info.selected_item);

	/* set blocking of scrolling */
	elm_scroller_movement_block_set(contextual_event_scroller, ELM_SCROLLER_MOVEMENT_BLOCK_HORIZONTAL);

	type = (missed_event_type_e)evas_object_data_get(item, LOCK_CONTEXTUAL_MISSED_EVENT_TYPE_KEY);
	_D("missed event type : %d", type);

	is_selected = (bool)evas_object_data_get(item, LOCK_CONTEXTUAL_MISSED_EVENT_IS_SELECTED_KEY);
	_D("selected : %d", is_selected);

	/* reset lcd off timer */
	lockscreen_lcd_off_timer_reset();

	if (!is_selected) {
		elm_object_part_text_set(contextual_event_layout, "txt.close", _("IDS_ST_BUTTON_CANCEL"));
		elm_object_signal_emit(item, "set,selected", "bg");
		s_info.selected_item = item;
		evas_object_data_set(item, LOCK_CONTEXTUAL_MISSED_EVENT_IS_SELECTED_KEY, (void *)true);

		lock_missed_event_item_disabled_set(EINA_TRUE);
	}
}

static void _missed_event_item_update(Evas_Object *item, char *icon_path, char *icon_sub_path, char *content, char *title, char *time, char *count_str, missed_event_type_e event_type)
{
	int lock_type = 0;
	int count = 0;

	char secure_content[256] = { 0, };

	ret_if(!item);

	lock_type = lockscreen_setting_lock_type_get();
	_D("lock type : %d", lock_type);

	/* update missed event icon */
	_missed_event_item_icon_update(item, icon_path, icon_sub_path, event_type);

	if (count_str) {
		count = atoi(count_str);
		_D("count : %d", count);
	}

	if (lock_type == SETTING_SCREEN_LOCK_TYPE_SIMPLE_PASSWORD ||
			lock_type == SETTING_SCREEN_LOCK_TYPE_PASSWORD) {
		switch(event_type) {
		case LOCK_MISSED_EVENT_TYPE_CALL :
			/* content */
			if (count == 1) {
				elm_object_part_text_set(item, "txt.content", _("IDS_AWGT_BODY_1_MISSED_CALL"));
			} else {
				snprintf(secure_content, sizeof(secure_content) - 1, _("IDS_LCKSCN_BODY_PD_MISSED_CALLS_T_TTS"), count);
				elm_object_part_text_set(item, "txt.content", secure_content);
			}

			/* title */
			elm_object_part_text_set(item, "txt.title", _("IDS_IDLE_POP_MISSED_CALL"));

			/* count */
			elm_object_part_text_set(item, "txt.variable", "");
			break;
		case LOCK_MISSED_EVENT_TYPE_MSG :
			/* content */
			if (count == 1) {
				elm_object_part_text_set(item, "txt.content", _("IDS_MSG_BODY_1_NEW_MESSAGE"));
			} else {
				snprintf(secure_content, sizeof(secure_content) - 1, _("IDS_LCKSCN_BODY_PD_NEW_MESSAGES_T_TTS"), count);
				elm_object_part_text_set(item, "txt.content", secure_content);
			}

			/* title */
			elm_object_part_text_set(item, "txt.title", _("IDS_MSGF_POP_NEW_MESSAGE"));

			/* count */
			elm_object_part_text_set(item, "txt.variable", "");
			break;
		default :
			_E("Failed to get missed event type. event type error(%d)", event_type);
			break;
		}
	} else {
		switch(event_type) {
		case LOCK_MISSED_EVENT_TYPE_CALL :
			elm_object_part_text_set(item, "txt.title", _("IDS_IDLE_POP_MISSED_CALL"));
			break;
		case LOCK_MISSED_EVENT_TYPE_MSG :
			if (title) {
				elm_object_part_text_set(item, "txt.title", title);
				_D("title : %s", title);
			}
			break;
		default :
			_E("Failed to get missed event type. event type error(%d)", event_type);
			break;
		}

		if (content) {
			elm_object_part_text_set(item, "txt.content", content);
			_D("content : %s", content);
		}

		if (count > 1) {
			elm_object_part_text_set(item, "txt.variable", count_str);
		} else {
			elm_object_part_text_set(item, "txt.variable", "");
		}
	}

	if (time) {
		elm_object_part_text_set(item, "txt.time", time);
		_D("time : %s", time);
	}
}

void lock_missed_event_item_destroy(Evas_Object *item)
{
	Evas_Object *icon = NULL;
	bundle *b = NULL;

	ret_if(!item);

	evas_object_data_del(item, LOCK_CONTEXTUAL_MISSED_EVENT_TYPE_KEY);
	evas_object_data_del(item, LOCK_CONTEXTUAL_MISSED_EVENT_IS_SELECTED_KEY);
	b = evas_object_data_del(item, LOCK_CONTEXTUAL_MISSED_EVENT_BUNDLE_KEY);
	if (b) {
		bundle_free(b);
	}

	icon = elm_object_part_content_unset(item, "sw.icon");
	if (icon) {
		_missed_event_item_icon_destroy(icon);
	}

	evas_object_del(item);
}


static Evas_Object *_missed_event_item_create(char *icon_path, char *icon_sub_path, char *content, char *title, char *time, char *count_str, missed_event_type_e event_type)
{
	Evas_Object *swipe_layout = NULL;
	Evas_Object *item = NULL;
	Evas_Object *icon = NULL;
	Evas_Object *item_btn = NULL;

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, NULL);

	item = elm_layout_add(swipe_layout);
	retv_if(!item, NULL);

	if (!elm_layout_file_set(item, LOCK_EDJE_FILE, "missed-event-item")) {
		_E("Failed to set edje file for missed event item");
		return NULL;
	}

	icon = _missed_event_item_icon_create(item, icon_path, icon_sub_path, event_type);
	if (!icon) {
		_E("Failed to create missed event icon(%s, %s)", icon_path, icon_sub_path);
	} else {
		elm_object_part_content_set(item, "sw.icon", icon);
	}

	_missed_event_item_update(item, icon_path, icon_sub_path, content, title, time, count_str, event_type);

	switch(event_type) {
	case LOCK_MISSED_EVENT_TYPE_CALL :
		elm_object_part_text_set(item, "txt.selected.message", _("IDS_LCKSCN_NPBODY_SWIPE_SCREEN_TO_VIEW_CALL_LOG_ABB"));
		break;
	case LOCK_MISSED_EVENT_TYPE_MSG :
		elm_object_part_text_set(item, "txt.selected.message", _("IDS_LCKSCN_NPBODY_SWIPE_SCREEN_TO_VIEW_MESSAGE_ABB"));
		break;
	default :
		_E("Failed to get missed event type. event type error(%d)", event_type);
		break;
	}

	item_btn = elm_button_add(item);
	if (!item_btn) {
		_E("Failed to create item button");

		evas_object_del(icon);
		icon = NULL;

		evas_object_del(item);
		item = NULL;

		return NULL;
	}
	elm_object_style_set(item_btn, "focus");
	elm_object_part_content_set(item, "btn.item", item_btn);
	evas_object_show(item_btn);

	evas_object_size_hint_weight_set(item, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(item, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_data_set(item, LOCK_CONTEXTUAL_MISSED_EVENT_TYPE_KEY, (void *)event_type);
	evas_object_data_set(item, LOCK_CONTEXTUAL_MISSED_EVENT_IS_SELECTED_KEY, (void *)false);

	evas_object_smart_callback_add(item_btn, "clicked", _missed_event_item_btn_clicked_cb, item);

	evas_object_show(item);

	return item;
}

static lock_error_e _notification_item_get(notification_h noti, char *pkgname)
{
	Evas_Object *item = NULL;

	char *str_count = NULL;
	char *ONE = "1";
	char *title = NULL;
	char *content = NULL;
	char *icon_path = NULL;
	char *icon_sub_path = NULL;
	char time[MAX_CONTENT];
	char *formatted_time = NULL;
	time_t noti_time;

	bundle *b = NULL;
	bundle *single_service_handle = NULL;

	missed_event_type_e type = LOCK_MISSED_EVENT_TYPE_NONE;

	retv_if(!noti, LOCK_ERROR_INVALID_PARAMETER);
	retv_if(!pkgname, LOCK_ERROR_INVALID_PARAMETER);

	retv_if(NOTIFICATION_ERROR_NONE != notification_get_text(noti, NOTIFICATION_TEXT_TYPE_TITLE, &title),
			LOCK_ERROR_FAIL);

	retv_if(NOTIFICATION_ERROR_NONE != notification_get_text(noti, NOTIFICATION_TEXT_TYPE_CONTENT, &content),
			LOCK_ERROR_FAIL);

	retv_if(NOTIFICATION_ERROR_NONE != notification_get_text(noti, NOTIFICATION_TEXT_TYPE_EVENT_COUNT, &str_count),
			LOCK_ERROR_FAIL);

	if (NOTIFICATION_ERROR_NONE != notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON, &icon_path)) {
		_E("Failed to get noti icon path : %s", pkgname);
	}

	if (NOTIFICATION_ERROR_NONE != notification_get_image(noti, NOTIFICATION_IMAGE_TYPE_ICON_SUB, &icon_sub_path)) {
		_E("Failed to get noti icon sub path : %s", pkgname);
	}

	retv_if(NOTIFICATION_ERROR_NONE != notification_get_time(noti, &noti_time), LOCK_ERROR_FAIL);

	formatted_time = lock_time_formatted_noti_time_get(noti_time);
	if (formatted_time) {
		strncpy(time, formatted_time, sizeof(time) - 1);
	}

	if (!str_count) {
		str_count = ONE;
	}

	if (!strncmp(APP_NAME_CALL, pkgname, strlen(APP_NAME_CALL)) ||
			!strncmp(APP_NAME_CONTACTS, pkgname, strlen(APP_NAME_CONTACTS))) {
		type = LOCK_MISSED_EVENT_TYPE_CALL;
		if (!icon_path) {
			icon_path = ICON_PATH_MISSED_EVENT_CALL;
		}
	} else if (!strncmp(APP_NAME_MSG, pkgname, strlen(APP_NAME_MSG))) {
		type = LOCK_MISSED_EVENT_TYPE_MSG;
		if (!icon_path) {
			icon_path = ICON_PATH_MISSED_EVENT_MSG;
		}
	}

	item = lock_contextual_event_missed_event_item_find(type);
	if (!item) {
		_D("create notification item(%s)", pkgname);

		item = _missed_event_item_create(icon_path, icon_sub_path, content, title, time, str_count, type);
		if (!item) {
			_E("Failed to create missed event item");
			free(formatted_time);
			return LOCK_ERROR_FAIL;
		}

		if (LOCK_ERROR_OK != lock_contextual_event_missed_event_item_add(item, type)) {
			_E("Failed to add missed event(%s)", pkgname);
		}
	} else {
		_D("update notification item(%s)", pkgname);
		_missed_event_item_update(item, icon_path, icon_sub_path, content, title, time, str_count, type);
	}

	if (NOTIFICATION_ERROR_NONE != notification_get_execute_option(noti, NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH, NULL, &b)) {
		_E("Failed to get bundle for missed event(%s)", pkgname);
	}

	if (b) {
		single_service_handle = bundle_dup(b);
		if (single_service_handle) {
			evas_object_data_set(item, LOCK_CONTEXTUAL_MISSED_EVENT_BUNDLE_KEY, (void *)single_service_handle);
		}
	}

	free(formatted_time);

	return LOCK_ERROR_OK;
}

static void _noti_changed_cb(void *data, notification_type_e type, notification_op *op_list, int num_op)
{
	Evas_Object *swipe_layout = NULL;
	char *pkgname = NULL;
	int app_list = 0;
	int need_create = 0;

	Eina_Bool is_call_exist = EINA_FALSE;
	Eina_Bool is_msg_exist = EINA_FALSE;

	swipe_layout = (Evas_Object *)data;
	ret_if(!swipe_layout);

	notification_list_h noti_list = NULL;
	notification_list_h noti_list_head = NULL;
	notification_error_e noti_err = NOTIFICATION_ERROR_NONE;

	notification_h noti = NULL;

	noti_err = notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &noti_list_head);
	ret_if(noti_err != NOTIFICATION_ERROR_NONE);

	noti_list = noti_list_head;
	while (noti_list != NULL) {
		need_create = 0;
		noti = notification_list_get_data(noti_list);
		notification_get_pkgname(noti, &pkgname);
		_D("noti pkgname : %s", pkgname);

		if (NOTIFICATION_ERROR_NONE != notification_get_display_applist(noti, &app_list)) {
			_E("Failed to get notification app list");
			return;
		}

		if (app_list & NOTIFICATION_DISPLAY_APP_LOCK) {
			_D("adding new notification");

			if (!strncmp(APP_NAME_CALL, pkgname, strlen(APP_NAME_CALL)) ||
					!strncmp(APP_NAME_CONTACTS, pkgname, strlen(APP_NAME_CONTACTS))) {
				need_create = 1;
				is_call_exist = EINA_TRUE;
			}

			if (!strncmp(APP_NAME_MSG, pkgname, strlen(APP_NAME_MSG))) {
				need_create = 1;
				is_msg_exist = EINA_TRUE;
			}

			if (need_create) {
				int ret = NOTIFICATION_ERROR_NONE;
				notification_h noti_clone = NULL;

				ret = notification_clone(noti, &noti_clone);
				if (ret != NOTIFICATION_ERROR_NONE) {
					_E("Failed to clone notification");
				} else {
					if (LOCK_ERROR_OK != _notification_item_get(noti_clone, pkgname)) {
						_E("Failed to add missed event item(%s)", pkgname);
					}

					notification_free(noti_clone);
				}
			}
		}
		noti_list = notification_list_get_next(noti_list);
	}

	if (!is_call_exist) {
		_E("delete missed event item(call)");
		lock_contextual_event_missed_event_item_del(LOCK_MISSED_EVENT_TYPE_CALL);
	}

	if (!is_msg_exist) {
		_E("delete missed event item(msg)");
		lock_contextual_event_missed_event_item_del(LOCK_MISSED_EVENT_TYPE_MSG);
	}

	notification_free_list(noti_list_head);
}

lock_error_e lock_missed_event_noti_cb_register(void)
{
	Evas_Object *swipe_layout = NULL;

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	notification_error_e ret = notification_register_detailed_changed_cb(_noti_changed_cb, (void *)swipe_layout);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("Failed to register notification cb(%d)", ret);
		return LOCK_ERROR_FAIL;
	}

	return LOCK_ERROR_OK;
}

void lock_missed_event_noti_cb_unregister(void)
{
	notification_error_e ret = notification_unregister_detailed_changed_cb(_noti_changed_cb, NULL);
	if (ret != NOTIFICATION_ERROR_NONE) {
		_E("Failed to unregister notification cb(%d)", ret);
	}
}

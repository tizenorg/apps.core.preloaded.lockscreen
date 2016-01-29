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

#include <telephony/telephony.h>

#include "lockscreen.h"
#include "log.h"
#include "sim_state.h"
#include "default_lock.h"

#define EMG_BUTTON_WIDTH 322

#define PLMN_LABEL_STYLE_START "<style=far_shadow,bottom><shadow_color=#00000033><font_size=24><align=left><color=#FFFFFF><text_class=ATO007><color_class=ATO007><wrap=none>"
#define PLMN_LABEL_STYLE_END "</wrap></color_class></text_class></color></align></font_size></shadow_color></style>"

static telephony_handle_list_s s_handle_list;

static void _sim_init(void);
static void _sim_deinit(void);

static void _sim_state_view_slide_mode_set(Evas_Object *label)
{
	Evas_Object *label_edje = NULL;
	Evas_Object *tb = NULL;
	Evas_Coord tb_w = 0;

	ret_if(!label);

	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);

	label_edje = elm_layout_edje_get(label);
	ret_if(!label_edje);

	tb = (Evas_Object *)edje_object_part_object_get(label_edje, "elm.text");
	ret_if(!tb);

	evas_object_textblock_size_native_get(tb, &tb_w, NULL);

	if ((tb_w > 0) && (tb_w > _X(EMG_BUTTON_WIDTH))) {
		elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	}

	elm_label_slide_go(label);
}

static void _sim_state_view_sliding_label_create(Evas_Object *layout, char *text)
{
	Evas_Object *label = NULL;
	char buf[512] = { 0, };
	char *markup_text = NULL;

	label = elm_label_add(layout);
	ret_if(!label);

	markup_text = elm_entry_utf8_to_markup(text);
	snprintf(buf, sizeof(buf), "%s%s%s", PLMN_LABEL_STYLE_START, markup_text, PLMN_LABEL_STYLE_END);
	free(markup_text);

	elm_object_style_set(label, "slide_short");
	elm_label_wrap_width_set(label, 100);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_label_slide_duration_set(label, 2);
	_sim_state_view_slide_mode_set(label);

	elm_object_text_set(label, buf);

	elm_object_part_content_set(layout, "txt.plmn", label);
	evas_object_show(label);
}

static char *_sim_plmn_get(telephony_h handle)
{
	int ret = 0;
	char *network_name = NULL;

	/* Reading Network (PLMN) name - ‘string’ type Property */
	ret = telephony_network_get_network_name(handle, &network_name);
	if (ret == TELEPHONY_ERROR_NONE) {
		/* ‘network_name’ contains valid Network name based on Display condition */
		return network_name;
	} else {
		_E("Sim = %p PLMN = ERROR[%d]", handle, ret);
		/* get property failed */
	}

	return NULL;
}

static char *_sim_spn_get(telephony_h handle)
{
	int ret = 0;
	char *spn_name = NULL;

	/* Reading SPN name - ‘string’ type Property */
	telephony_sim_get_spn(handle, &spn_name);
	if (ret == TELEPHONY_ERROR_NONE) {
		/* ‘spn_name’ contains valid Service provider name */
		return spn_name;
	} else {
		_E("Sim = %p SPN = ERROR[%d]", handle, ret);
		/* get property failed */
		return NULL;
	}
}

static char *_sim_state_text_for_sim_get(telephony_h handle)
{
	int ret;
	telephony_network_service_state_e service_state;
	telephony_network_name_option_e name_option;

	char *plmn = NULL;
	char *spn = NULL;
	char buf[1024] = { 0, };

	/* get service state */
	ret = telephony_network_get_service_state(handle, &service_state);
	if (ret != TELEPHONY_ERROR_NONE) {
		_E("Failed to get service state [%d]", ret);
		return NULL;
	}

	switch (service_state) {
	case TELEPHONY_NETWORK_SERVICE_STATE_IN_SERVICE:
		/* get network name option */
		ret = telephony_network_get_network_name_option(handle, &name_option);
		if (ret != TELEPHONY_ERROR_NONE) {
			_E("Failed to get telephony network name option [%d]", ret);
			return NULL;
		}

		switch (name_option) {
		case TELEPHONY_NETWORK_NAME_OPTION_SPN:
			spn = _sim_spn_get(handle);
			if (spn != NULL && spn[0] != 0) {
				_I("PLMN/SPN - Sim %p using SPN: %s", handle, spn);
				snprintf(buf, sizeof(buf), "%s", spn);
			}
			break;
		case TELEPHONY_NETWORK_NAME_OPTION_NETWORK:
			plmn = _sim_plmn_get(handle);
			if (plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		case TELEPHONY_NETWORK_NAME_OPTION_ANY:
			spn = _sim_spn_get(handle);
			plmn = _sim_plmn_get(handle);
			if (spn != NULL && spn[0] != 0 && plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using SPN: %s, PLMN: %s", handle, spn, plmn);
				snprintf(buf, sizeof(buf), "%s - %s", plmn, spn);
			} else if (spn != NULL && spn[0] != 0) {
				_I("PLMN/SPN - Sim %p using SPN: %s", handle, spn);
				snprintf(buf, sizeof(buf), "%s", spn);
			} else if (plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		default:
			_E("Invalid name option[%d]", name_option);
			plmn = _sim_plmn_get(handle);
			if (plmn != NULL && plmn[0] != 0) {
				_I("PLMN/SPN - Sim %p using PLMN: %s", handle, plmn);
				snprintf(buf, sizeof(buf), "%s", plmn);
			}
			break;
		}
		break;
	case TELEPHONY_NETWORK_SERVICE_STATE_OUT_OF_SERVICE:
		snprintf(buf, sizeof(buf), "%s", _("IDS_COM_BODY_NO_SERVICE"));
		break;
	case TELEPHONY_NETWORK_SERVICE_STATE_EMERGENCY_ONLY:
		snprintf(buf, sizeof(buf), "%s", _("IDS_IDLE_MBODY_EMERGENCY_CALLS_ONLY"));
		break;
	default:
		snprintf(buf, sizeof(buf), "%s", _("IDS_COM_BODY_NO_SERVICE"));
		break;
	}

	return strdup(buf);
}

static void _sim_state_view_update()
{
	Evas_Object *swipe_layout = lock_default_swipe_layout_get();
	ret_if(!swipe_layout);
	char *sim1 = NULL, *sim2 = NULL;
	char buf[1024];

	switch (s_handle_list.count) {
		case 1:
			sim1 = _sim_state_text_for_sim_get(s_handle_list.handle[0]);
			_sim_state_view_sliding_label_create(swipe_layout, sim1);
			break;
		case 2:
			sim1 = _sim_state_text_for_sim_get(s_handle_list.handle[0]);
			sim2 = _sim_state_text_for_sim_get(s_handle_list.handle[1]);
			snprintf(buf, sizeof(buf), "%s / %s", sim1, sim2);
			_sim_state_view_sliding_label_create(swipe_layout, buf);
			break;
		default:
			_E("Unsupported sim card number (!= 1 && != 2).");
			return;
	}

	free(sim1);
	free(sim2);
}

static void _on_sim_card_info_changed_cb(telephony_h handle, telephony_noti_e noti_id, void *data, void *user_data)
{
	_sim_state_view_update();
}

static void _sim_callbacks_register(telephony_h handle)
{
	int ret = 0;

	ret = telephony_set_noti_cb(handle, TELEPHONY_NOTI_SIM_STATUS, _on_sim_card_info_changed_cb, NULL);
	if (ret != TELEPHONY_ERROR_NONE) {
		_E("Failed to register callback on event TELEPHONY_NOTI_SIM_STATUS [%d]", ret);
	}

	ret = telephony_set_noti_cb(handle, TELEPHONY_NOTI_NETWORK_NETWORK_NAME, _on_sim_card_info_changed_cb, NULL);
	if (ret != TELEPHONY_ERROR_NONE) {
		_E("Failed to register callback on event TELEPHONY_NOTI_NETWORK_NETWORK_NAME [%d]", ret);
	}

	ret = telephony_set_noti_cb(handle, TELEPHONY_NOTI_NETWORK_SERVICE_STATE, _on_sim_card_info_changed_cb, NULL);
	if (ret != TELEPHONY_ERROR_NONE) {
		_E("Failed to register callback on event TELEPHONY_NOTI_NETWORK_SERVICE_STATE [%d]", ret);
	}
}

static void _on_telephony_state_changed_cb(telephony_state_e state, void *user_data)
{
	_D("tel status[%d]", state);

	if (state == TELEPHONY_STATE_READY) {
		_sim_init();
		_sim_state_view_update();
	} else if (state == TELEPHONY_STATE_NOT_READY) {
		_sim_deinit();
	}
}

/**
 * @brief Initializes SIM card handlers
 */
static void _sim_init(void)
{
	/* Get available sim cards handles */
	int i, ret = telephony_init(&s_handle_list);
	if (ret != TELEPHONY_ERROR_NONE) {
		_E("Unable to initialize telephony");
		return;
	}

	if (s_handle_list.count <= 0) {
		_D("No available sim cards");
		return;
	}

	/* Register for Telephony call state changes */
	for (i = 0; i < s_handle_list.count; i++) {
		_sim_callbacks_register(s_handle_list.handle[i]);
	}
}

static void _sim_deinit(void)
{
	int ret = telephony_deinit(&s_handle_list);
	if (ret != TELEPHONY_ERROR_NONE) {
		_E("Unable to deinit telephony list");
	}
	s_handle_list.count = 0;
}

lock_error_e lock_sim_state_init(void)
{
	int ret;
	telephony_state_e state;

	/* Check if Telephony state == READY */
	ret = telephony_get_state(&state);
	if (ret == TELEPHONY_ERROR_NONE) {
		if (state == TELEPHONY_STATE_READY) {
			_D("Telephony Ready : %d", state);
			_sim_init();
			_sim_state_view_update();
		} else if (state == TELEPHONY_STATE_NOT_READY) {
			_D("Telephony state: [NOT Ready]");
		}
	} else {
		_E("Unable to get telephony state: %d", ret);
	}

	/* Register for Telephony state change */
	ret = telephony_set_state_changed_cb(_on_telephony_state_changed_cb, NULL);
	if (ret != TELEPHONY_ERROR_NONE) {
		_E("Failed to register on telephony state changed callback");
	}

	return LOCK_ERROR_OK;
}

void lock_sim_state_deinit(void)
{
	_D("De-initialization");
	_sim_deinit();

	telephony_unset_state_changed_cb(_on_telephony_state_changed_cb);
}

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

#include <device/battery.h>
#include <device/callback.h>

#include "lockscreen.h"
#include "log.h"
#include "battery_ctrl.h"
#include "default_lock.h"
#include "data_model.h"

#include <Ecore.h>

static Ecore_Event_Handler *handler;


// FIXME why this is needed?
static char *_replaceString(char *strInput, const char *strTarget, const char *strChange)
{
	char* strResult;
	char* strTemp;
	int i = 0;
	int nCount = 0;
	int nTargetLength = strlen(strTarget);

	if (nTargetLength < 1) {
		_E("there is no target to chnage");
		return NULL;
	}

	int nChangeLength = strlen(strChange);

	if (nChangeLength != nTargetLength) {
		for (i = 0; strInput[i] != '\0';) {
			if (memcmp(&strInput[i], strTarget, nTargetLength) == 0) {
				nCount++;		//consider same string exist
				i += nTargetLength;
			} else {
				i++;
			}
		}
	} else {
		i = strlen(strInput);
	}

	strResult = (char *) malloc(i + 1 + nCount * (nChangeLength - nTargetLength));

	if (!strResult) {
		_E("fail malloc!!");
		return NULL;
	}

	strTemp = strResult;
	while (*strInput) {
		if (memcmp(strInput, strTarget, nTargetLength) == 0) {
			memcpy(strTemp, strChange, nChangeLength);
			strTemp += nChangeLength;	//move changed length
			strInput  += nTargetLength;	// move target length
		} else {
			*strTemp++ = *strInput++;		// original str cpy
		}
	}

	*strTemp = '\0';

	return strResult;
}

static char *_text_from_percentage(int capacity)
{
	char buff[64];
	char *newString = NULL;
	newString = _replaceString(_("IDS_LCKSCN_BODY_CHARGING_C_PDP"), "%d%", "%d%%");

	if (newString != NULL) {
		snprintf(buff, sizeof(buff), newString , capacity);
		free(newString) ;
	} else {
		snprintf(buff, sizeof(buff), _("IDS_LCKSCN_BODY_CHARGING_C_PDP") , capacity);
	}
	return strdup(buff);
}

lock_error_e lock_battery_update(void)
{
	Evas_Object *swipe_layout = NULL;
	const lockscreen_data_model_t *model = lockscreen_data_model_get_model();

	if (!model) {
		_E("Lockscreen data model not available");
		elm_object_signal_emit(swipe_layout, "hide,txt,battery", "txt.battery");
		return LOCK_ERROR_FAIL;
	}

	swipe_layout = lock_default_swipe_layout_get();
	retv_if(!swipe_layout, LOCK_ERROR_FAIL);

	if (model->battery.is_connected) {
		elm_object_signal_emit(swipe_layout, "show,txt,battery", "txt.battery");
	} else {
		elm_object_signal_emit(swipe_layout, "hide,txt,battery", "txt.battery");
	}

	if (model->battery.is_charging) {
		if (model->battery.level == 100) {
			elm_object_part_text_set(swipe_layout, "txt.battery", _("IDS_SM_POP_FULLY_CHARGED"));
		} else {
			char *buff = _text_from_percentage(model->battery.level);
			elm_object_part_text_set(swipe_layout, "txt.battery", buff);
			free(buff);
		}
	} else {
		if (model->battery.level == 100 && model->battery.is_connected) {
			elm_object_part_text_set(swipe_layout, "txt.battery", _("IDS_SM_POP_FULLY_CHARGED"));
		} else {
			elm_object_part_text_set(swipe_layout, "txt.battery", "");
		}
	}
	return LOCK_ERROR_OK;
}

static Eina_Bool _battery_update(void *data, int event, void *event_info)
{
	lock_battery_update();
	return EINA_TRUE;
}

lock_error_e lock_battery_ctrl_init(void)
{
	handler = ecore_event_handler_add(LOCKSCREEN_DATA_MODEL_EVENT_BATTERY_CHANGED, _battery_update, NULL);
	if (!handler)
	{
		_E("Failed to register on EVENT_BATTERY_CHAGNED event");
		return LOCK_ERROR_FAIL;
	}
	lock_battery_update();

	return LOCK_ERROR_OK;
}

void lock_battery_ctrl_fini(void)
{
	ecore_event_handler_del(handler);
}

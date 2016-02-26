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

#include "lockscreen.h"
#include "log.h"
#include "sim_state.h"
#include "default_lock.h"
#include "data_model.h"

static Ecore_Event_Handler *handler;

#define EMG_BUTTON_WIDTH 322

#define PLMN_LABEL_STYLE_START "<style=far_shadow,bottom><shadow_color=#00000033><font_size=24><align=left><color=#FFFFFF><text_class=ATO007><color_class=ATO007><wrap=none>"
#define PLMN_LABEL_STYLE_END "</wrap></color_class></text_class></color></align></font_size></shadow_color></style>"

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

static void _sim_state_view_sliding_label_create(Evas_Object *layout, const char *text)
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

static void _sim_state_view_update()
{
	Evas_Object *swipe_layout = lock_default_swipe_layout_get();
	ret_if(!swipe_layout);
	char buf[128];

	const lockscreen_data_model_t *model = lockscreen_data_model_get_model();
	if (!model) {
		_E("lockscreen_data_model_get_model failed");
		return;
	}

	if (model->sim[0].text && model->sim[1].text) {
		snprintf(buf, sizeof(buf), "%s / %s", model->sim[0].text, model->sim[1].text);
	} else if (model->sim[0].text) {
		snprintf(buf, sizeof(buf), "%s", model->sim[0].text);
	} else if (model->sim[1].text) {
		snprintf(buf, sizeof(buf), "%s", model->sim[1].text);
	} else {
		snprintf(buf, sizeof(buf), "");
	}

	_sim_state_view_sliding_label_create(swipe_layout, buf);
}

static Eina_Bool _sim_status_changed(void *data, int type, void *event_info)
{
	_sim_state_view_update();
	return EINA_TRUE;
}

lock_error_e lock_sim_state_init(void)
{
	handler = ecore_event_handler_add(LOCKSCREEN_DATA_MODEL_EVENT_SIM_STATUS_CHANGED, _sim_status_changed, NULL);

	return LOCK_ERROR_OK;
}

void lock_sim_state_deinit(void)
{
	_D("De-initialization");
	ecore_event_handler_del(handler);
}

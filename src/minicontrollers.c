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

#include <minicontrol-viewer.h>
#include <Ecore.h>

#include "log.h"
#include "minicontrollers.h"

static const char *active_minicontroller;
static int init_count, width, height;
int LOCKSCREEN_EVENT_MINICONTROLLER_CHANGED;

static void _minicontroller_start_handle(const char *name, int w, int h)
{
	if (!active_minicontroller && strstr(name, "LOCKSCREEN]")) {
		active_minicontroller = eina_stringshare_add(name);
		width = w;
		height = h;
		ecore_event_add(LOCKSCREEN_EVENT_MINICONTROLLER_CHANGED, NULL, NULL, NULL);
	}
}

static void _minicontroller_stop_handle(const char *name)
{
	if (active_minicontroller && !strcmp(name, active_minicontroller)) {
		eina_stringshare_del(active_minicontroller);
		active_minicontroller = NULL;
		ecore_event_add(LOCKSCREEN_EVENT_MINICONTROLLER_CHANGED, NULL, NULL, NULL);
	}
}

static void _minicontroller_geometry_from_bundle_get(bundle *event_arg, int *width, int *height)
{
	int *val;
	size_t val_size;
	if (!event_arg)
		return;
	int ret = bundle_get_byte(event_arg, "width", (void**)&val, &val_size);
	if (ret == BUNDLE_ERROR_NONE) {
		if (width) *width = *val;
	}
	ret = bundle_get_byte(event_arg, "height", (void**)&val, &val_size);
	if (ret == BUNDLE_ERROR_NONE) {
		if (height) *height = *val;
	}
}

static void _minicontroler_event(minicontrol_event_e event, const char *minicontrol_name, bundle *event_arg, void *data)
{
	int w = 0, h = 0;
	if (!minicontrol_name)
		return;

	_D("Available minicontroller: %s", minicontrol_name);

	switch (event) {
		case MINICONTROL_EVENT_START:
			_minicontroller_geometry_from_bundle_get(event_arg, &w, &h);
			_minicontroller_start_handle(minicontrol_name, w, h);
			break;
		case MINICONTROL_EVENT_STOP:
			_minicontroller_stop_handle(minicontrol_name);
			break;
		default:
			_D("Unahandled minicontroller event: %d for %s", event, minicontrol_name);
			break;
	}
}

int lockscreen_minicontrollers_init(void)
{
	if (!init_count) {
		LOCKSCREEN_EVENT_MINICONTROLLER_CHANGED = ecore_event_type_new();
		int ret = minicontrol_viewer_set_event_cb(_minicontroler_event, NULL);
		if (ret != MINICONTROL_ERROR_NONE) {
			_E("minicontrol_viewer_set_event_cb failed: %s", get_error_message(ret));
			return 1;
		}
	}
	init_count++;
	return 0;
}

void lockscreen_minicontrollers_shutdown(void)
{
	if (init_count) {
		init_count--;
		if (!init_count) {
			int ret = minicontrol_viewer_unset_event_cb();
			if (ret != MINICONTROL_ERROR_NONE) {
				_E("minicontrol_viewer_unset_event_cb failed: %s", get_error_message(ret));
			}
			eina_stringshare_del(active_minicontroller);
		}
	}
}

Evas_Object *lockscreen_minicontrollers_active_minicontroller_get(Evas_Object *parent)
{
	if (!active_minicontroller) return NULL;
	Evas_Object *ret = minicontrol_viewer_add(parent, active_minicontroller);
	evas_object_size_hint_min_set(ret, width, height);
	_D("Create minicontroller for %s, (size: %d %d)", active_minicontroller, width, height);
	evas_object_show(ret);
	return ret;
}

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

#include <Elementary.h>
#include <minicontrol-viewer.h>

#include "log.h"
#if 0

#include "lockscreen.h"
#include "log.h"
#include "music_player.h"
#include "background_view.h"
#include "window.h"

#define MUSIC_INI_DIR "/opt/usr/apps/org.tizen.music-player/data"
#define MUSIC_PLAYING_FILE_NAME "playing_track_music.ini"
#define SOUND_PLAYING_FILE_NAME "playing_track_sound.ini"
#define MUSIC_PLAYING_INI_FILE MUSIC_INI_DIR"/"MUSIC_PLAYING_FILE_NAME
#define SOUND_PLAYING_INI_FILE MUSIC_INI_DIR"/"SOUND_PLAYING_FILE_NAME
#define PLAYER_STATUS "/opt/usr/apps/org.tizen.music-player/shared/data/MusicPlayStatus.ini"
#define BUF_LEN 1024
#define NULL_FILE_NAME "(null)"
#define DEFAULT_THUMB_FILE_NAME "/opt/usr/share/media/.thumb/thumb_default.png"

static struct _s_info {
	Ecore_File_Monitor *music_file_monitor;
	Evas_Object *music_minicontroller;
	music_state_e music_state;
} s_info = {
	.music_file_monitor = NULL,
	.music_minicontroller = NULL,
	.music_state = MUSIC_STATE_NO_MUSIC,
};

music_state_e lock_music_player_state_get(void)
{
	return s_info.music_state;
}

Evas_Object *lock_music_player_minicontroller_get(void)
{
	return s_info.music_minicontroller;
}

static char *_minictrl_music_album_file_name_get(read_file_error_e *error, const char *file_name)
{
	FILE *fp = NULL;
	char line[BUF_LEN] = {0,};
	char *buf = NULL;
	int i = 0;

	if (!(fp = fopen(file_name, "r"))) {
		_E("Failed to open %s", file_name);
		return NULL;
	}

	for (i = 0; i < 3; i++) {
		if (!fgets(line, BUF_LEN-1, fp)) {
			_E("Failed to read player file : %d %d", i, ferror(fp));
			if (ferror(fp)) {
				*error = READ_FILE_ERROR_ERROR;
			}
			if (feof(fp)) {
				*error = READ_FILE_ERROR_EOF;
			}
			fclose(fp);
			return NULL;
		}
	}

	if (fclose(fp) < 0) {
		_E("Failed to close fp");
	}

	buf = strdup(line);
	if (!buf) {
		_E("Failed to allocate memory");
		return NULL;
	}

	if (!strncmp(buf, NULL_FILE_NAME, strlen(NULL_FILE_NAME))) {
		free(buf);
		return NULL;
	}

	int it = 0;

	while(buf[it] != '\0') {
		it++;
	}

	buf[it - 1] = '\0';

	return buf;
}

static void _album_art_bg_set(char *file_name)
{
	read_file_error_e error = READ_FILE_ERROR_NONE;
	char *file = _minictrl_music_album_file_name_get(&error, file_name);
	_D("file : %s", file);
	if (error != READ_FILE_ERROR_NONE) {
		_E("Error during read music info file(%d)", error);
		return;
	}

	if (!file) {
		if (LOCK_ERROR_OK != lock_background_view_image_set(LOCK_BG_DEFAULT, NULL)) {
			_E("Failed to set a default BG image");
		}
		return;
	}

	if (!strcmp(file, DEFAULT_THUMB_FILE_NAME)) {
		if (LOCK_ERROR_OK != lock_background_view_image_set(LOCK_BG_DEFAULT, NULL)) {
			_E("Failed to set a default BG image");
		}
		free(file);
		return;
	}

	if (LOCK_ERROR_OK != lock_background_view_image_set(LOCK_BG_ALBUM_ART, file)) {
		_E("Failed to set a album art BG image");
	}

	free(file);
}

static void _file_monitor_cb(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
{
	ret_if(!data);

	if (event == ECORE_FILE_EVENT_MODIFIED) {
		_album_art_bg_set((char*)data);
	}
}

void lock_music_player_bg_set(Evas_Object *bg, music_state_e music_state)
{
	char *file_name = NULL;
	if (!s_info.music_file_monitor) {
		if (music_state == MUSIC_STATE_MUSIC_PLAYER_ON) {
			file_name = MUSIC_PLAYING_INI_FILE;
		} else {
			file_name = SOUND_PLAYING_INI_FILE;
		}
		s_info.music_file_monitor = ecore_file_monitor_add(file_name, _file_monitor_cb, file_name);
	}

	if (file_name) {
		_album_art_bg_set(file_name);
	}
}

void lock_music_player_bg_unset(void)
{
	if (s_info.music_file_monitor) {
		ecore_file_monitor_del(s_info.music_file_monitor);
		s_info.music_file_monitor = NULL;
	}

	if (LOCK_ERROR_OK != lock_background_view_image_set(LOCK_BG_DEFAULT, NULL)) {
		_E("Failed to set a default BG image");
	}
}

bool music_player_is_playing(void)
{
	FILE *fp = NULL;
	char line[BUF_LEN] = {0, };
	bool ret = false;

	if (!(fp = fopen(PLAYER_STATUS, "r"))) {
		_E("Failed to open music player status file!");
		return false;
	}

	if (fgets(line, BUF_LEN, fp)) {
		if (strstr(line, "play")) {
			ret = true;
		}
	}

	fclose(fp);
	return ret;
}

void lock_music_player_minicontroller_hide_event_send(void)
{
	char *minicontrol_name = NULL;

	ret_if(!s_info.music_minicontroller);

	minicontrol_name = evas_object_data_get(s_info.music_minicontroller, LOCK_MUSIC_PLAYER_MINICONTROL_NAME_KEY);
	ret_if(!minicontrol_name);

	minicontrol_viewer_send_event(minicontrol_name, MINICONTROL_EVENT_REQUEST_HIDE, NULL);
}

Evas_Object *lock_music_player_minicontroller_create(music_state_e state, Evas_Object *layout, const char *name)
{
	Evas_Object *mini_control = NULL;

	mini_control = minicontrol_viewer_add(layout, name);
	retv_if(!mini_control, NULL);

	_D("music state : %d", state);
	s_info.music_state = state;

	evas_object_size_hint_weight_set(mini_control, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(mini_control, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_data_set(mini_control, LOCK_MUSIC_PLAYER_MINICONTROL_NAME_KEY, (void *)name);

	evas_object_show(mini_control);

	s_info.music_minicontroller = mini_control;

	return mini_control;
}

void lock_music_player_minicontroller_destroy(void)
{
	lock_music_player_bg_unset();

	s_info.music_state = MUSIC_STATE_NO_MUSIC;

	if (s_info.music_minicontroller) {
		evas_object_data_del(s_info.music_minicontroller, LOCK_MUSIC_PLAYER_MINICONTROL_NAME_KEY);
		evas_object_del(s_info.music_minicontroller);
		s_info.music_minicontroller = NULL;
	}
}
///////////////////////////////////////////////////

#endif

#include "music_player.h"
#include "util.h"

static int _init_count;
static player_state_cb callback;
static void *user_data;

typedef struct {
	const char *name;
	music_player_view_type_e type;
	bool state;
} view_status_t;

static view_status_t states[] = {
	{"[music-minicontrol.LOCKSCREEN]", MUSIC_PLAYER_VIEW_TYPE_MUSIC, false},
	{"[sound-minicontrol.LOCKSCREEN]", MUSIC_PLAYER_VIEW_TYPE_SOUND, false},
};

static view_status_t *_status_get_by_name(const char *name)
{
	int i;
	for (i = 0; i < SIZE(states); i++)
	{
		if (!strcmp(name, states[i].name))
			return &states[i];
	}
	return NULL;
}

static view_status_t *_status_get_by_type(music_player_view_type_e type)
{
	int i;
	for (i = 0; i < SIZE(states); i++)
	{
		if (type == states[i].type)
			return &states[i];
	}
	return NULL;
}

static void _music_player_minicontroller_start_handle(const char *name)
{
	view_status_t *status = _status_get_by_name(name);
	if (!status) {
		_E("Unhandled minicontroller name: %s", name);
		return;
	}

	if (!status->state) {
		status->state = true;
		if (callback) callback(status->type, true, user_data);
	}
}

static void _music_player_minicontroller_stop_handle(const char *name)
{
	view_status_t *status = _status_get_by_name(name);
	if (!status) {
		_E("Unhandled minicontroller name: %s", name);
		return;
	}

	if (status->state) {
		status->state = false;
		if (callback) callback(status->type, false, user_data);
	}
}

Evas_Object *lock_music_player_view_create(music_player_view_type_e type, Evas_Object *parent)
{
	view_status_t *status = _status_get_by_type(type);
	if (!status->state)
	{
		_E("Minicontroler view is not enabled");
		return NULL;
	}
	return minicontrol_viewer_add(parent, status->name);
}

static void _music_player_minicontroler_event(minicontrol_event_e event, const char *minicontrol_name, bundle *event_arg, void *data)
{
	if (!minicontrol_name)
		return;

	_D("Minicontroller: %s", minicontrol_name);

	switch (event) {
		case MINICONTROL_EVENT_START:
			_music_player_minicontroller_start_handle(minicontrol_name);
			break;
		case MINICONTROL_EVENT_STOP:
			_music_player_minicontroller_stop_handle(minicontrol_name);
			break;
		default:
			_D("Unahandled minicontroller event: %d for %s", event, minicontrol_name);
			break;
	}
}

int lock_music_player_init(void)
{
	if (!_init_count++) {
		int ret = minicontrol_viewer_set_event_cb(_music_player_minicontroler_event, NULL);
		if (ret != MINICONTROL_ERROR_NONE) {
			_E("minicontrol_viewer_set_event_cb failed: %s", get_error_message(ret));
			_init_count = 0;
		}
	}
	return _init_count;
}

void lock_music_player_shutdown(void)
{
	if (_init_count) {
		_init_count--;
		if (!_init_count) {
			int ret = minicontrol_viewer_unset_event_cb();
			if (ret != MINICONTROL_ERROR_NONE) {
				_E("minicontrol_viewer_unset_event_cb failed: %s", get_error_message(ret));
			}
			lock_music_player_state_changed_cb(NULL, NULL);
		}
	}
}

int lock_music_player_state_changed_cb(player_state_cb cb, void *data)
{
	callback = cb;
	user_data = data;

	return 0;
}

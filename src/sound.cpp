/*
 * sound.cpp
 * Copyright (C) 2012 Daniel Santos
 * Copyright (C) 2019 R. Danbrook
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#include <string>
#include <vector>

#include "cmixer.h"

#include "sound.h"
#include "config.h"
#include "error.h"
#include "settings.h"
#include "u4file.h"

using std::string;
using std::vector;

static cm_Source *effect[SOUND_MAX];

void xu4_snd_play(int sound, bool onlyOnce, int specificDurationInTicks) {
	if (sound >= SOUND_MAX) {
		xu4_error(XU4_LOG_WRN, "Tried to play an invalid sound!");
		return;
	}
	else if (!settings.soundVol) { return; }
	else if (effect[sound] == NULL) { return; }
	
	cm_play(effect[sound]);
}

void xu4_snd_stop() {
	for (int i = 0; i < SOUND_MAX; i++) {
		cm_stop(effect[i]);
	}
}

void xu4_snd_vol(double volume) {
	// Every source has to be done independently
    for (int i = 0; i < SOUND_MAX; i++) {
		cm_set_gain(effect[i], volume);
	}
}

int xu4_snd_vol_inc() {
	if (++settings.soundVol > MAX_VOLUME) { settings.soundVol = MAX_VOLUME; }
	else { xu4_snd_vol((double)settings.soundVol / MAX_VOLUME); }
	return (settings.soundVol * MAX_VOLUME);
}

int xu4_snd_vol_dec() {
	if (--settings.soundVol < 0) { settings.soundVol = 0; }
	else { xu4_snd_vol((double)settings.soundVol / MAX_VOLUME); }
	return (settings.soundVol * MAX_VOLUME);
}

static void xu4_snd_free_files() {
	for (int i = 0; i < SOUND_MAX; i++) {
		if (effect[i]) { free(effect[i]); }
	}
}

static void xu4_snd_load_files() {
	const Config *config = Config::getInstance();
	
	vector<ConfigElement> soundConfs = config->getElement("sound").getChildren();
	std::vector<ConfigElement>::const_iterator i = soundConfs.begin();
	std::vector<ConfigElement>::const_iterator theEnd = soundConfs.end();
	
	for (; i != theEnd; ++i) { // FIXME handle failure to read file
		if (i->getName() != "track") { continue; }
		int j = (i - soundConfs.begin()); // major hack while converting away from C++
		effect[j] = cm_new_source_from_file(u4find_sound(i->getString("file")).c_str());
	}
}

void xu4_snd_init() {
	xu4_snd_load_files();
	xu4_snd_vol((double)settings.soundVol / MAX_VOLUME);
}

void xu4_snd_deinit() {
	xu4_snd_free_files();
}

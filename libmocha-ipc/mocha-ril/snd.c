/**
 * This file is part of mocha-ril.
 *
 * Copyright (C) 2011 Paul Kocialkowski <contact@oaulk.fr>
 *
 * mocha-ril is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mocha-ril is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mocha-ril.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define LOG_TAG "RIL-Mocha-SND"
#include <utils/Log.h>

#include "mocha-ril.h"
#include "util.h"
#include <sound.h>


void ril_request_set_mute(RIL_Token t, void *data, size_t datalen)
{
	unsigned char mute_data = ((int *)data)[0] > 0 ? 1 : 0;
	ALOGD("Mute data is %d\n", mute_data);
	sound_send_set_mute(SND_INPUT_MIC, SND_OUTPUT_2, mute_data, mute_data, SND_TYPE_VOICE);
	
	ril_request_complete(t, RIL_E_SUCCESS, NULL, 0);
}

void srs_snd_set_volume(struct srs_message *message)
{
	struct srs_snd_set_volume_packet *volume = (struct srs_snd_set_volume_packet *) message->data;

	ALOGD("Volume for: 0x%x vol = 0x%x\n", volume->soundType, volume->volume);
	sound_send_set_volume(SND_OUTPUT_2 /* We should lookup for it by active soundtype */, 0, 0, 
				volume->soundType, volume->volume /* conversion might be needed */);
}

void srs_snd_set_audio_path(struct srs_message *message)
{
	struct srs_snd_set_path_packet *set_path = (struct srs_snd_set_path_packet *) message->data;

	ALOGD("srs_snd_set_audio_path - sndType: %d, indev: %d, outdev: %d\n", set_path->soundType, set_path->inDevice, set_path->outDevice);
	sound_send_set_path(set_path->inDevice, set_path->outDevice, 0, 0, set_path->soundType, 6 /* dummy volume */);
	sound_send_set_mute(set_path->inDevice, set_path->outDevice, 0, 0, set_path->soundType);
}

/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include "audio_status.h"

audio_status status;

void audio_status_init()
{
    status.volume = 0.0;
    status.muted = TRUE;
}

void audio_status_destroy()
{
}

audio_status *shared_audio_status()
{
    return &status;
}

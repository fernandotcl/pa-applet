/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#ifndef AUDIO_STATUS_H
#define AUDIO_STATUS_H

#include <glib.h>

typedef struct {
    gdouble volume;
    gboolean muted;
} audio_status;

void audio_status_init();
void audio_status_destroy();
audio_status *shared_audio_status();

#endif

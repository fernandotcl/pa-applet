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
#include <stdint.h>

typedef struct {
    gdouble volume;
    gboolean muted;
    GSList *profiles;
} audio_status;

typedef struct {
    gchar *name;
    gchar *description;
    uint32_t priority;
    gboolean active;
} audio_status_profile;

void audio_status_init();
void audio_status_destroy();
audio_status *shared_audio_status();
void audio_status_reset_profiles();
void audio_status_sort_profiles();

#endif

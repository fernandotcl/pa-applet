/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#define STATUS_STEP_SIZE 5.0

#include "audio_status.h"

audio_status status;

void audio_status_init(void)
{
    status.volume = 0.0;
    status.muted = TRUE;
}

void audio_status_destroy(void)
{
    audio_status_reset_profiles();
}

audio_status *shared_audio_status(void)
{
    return &status;
}

static void profile_destroy(gpointer *data)
{
    audio_status_profile *profile = (audio_status_profile *)data;
    g_free(profile->name);
    g_free(profile->description);
    g_free(profile);
}

void audio_status_raise_volume(void)
{
    status.volume += STATUS_STEP_SIZE;
    if (status.volume > 150.0)
        status.volume = 150.0;
}

void audio_status_lower_volume(void)
{
    status.volume -= STATUS_STEP_SIZE;
    if (status.volume < 0.0)
        status.volume = 0.0;
}

void audio_status_toggle_muted(void)
{
    status.muted = !status.muted;
}

void audio_status_reset_profiles(void)
{
    if (status.profiles) {
        g_slist_free_full(status.profiles, (GDestroyNotify)profile_destroy);
        status.profiles = NULL;
    }
}

static gint profile_compare_func(gconstpointer a, gconstpointer b)
{
    audio_status_profile *profile_a = (audio_status_profile *)a;
    audio_status_profile *profile_b = (audio_status_profile *)b;
    if (profile_a->priority > profile_b->priority)
        return -1;
    else if (profile_b->priority > profile_a->priority)
        return 1;
    else
        return 0;
}

void audio_status_sort_profiles(void)
{
    if (status.profiles)
        status.profiles = g_slist_sort(status.profiles, profile_compare_func);
}

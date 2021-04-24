/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <pulse/glib-mainloop.h>
#include <pulse/pulseaudio.h>

#include "audio_status.h"
#include "popup_menu.h"
#include "pulse_glue.h"
#include "tray_icon.h"
#include "volume_scale.h"

static pa_context *context;
static pa_glib_mainloop *loop;
static pa_mainloop_api *api;

static gboolean subscribed = FALSE;
static gboolean have_default_card_index = FALSE;
static uint32_t default_card_index;
static uint32_t default_sink_index;
static unsigned int default_sink_num_channels;

static pa_operation *sink_reload_operation = NULL;
static guint postponed_sink_reload_timeout_id;
static gboolean has_postponed_sink_reload = FALSE;

static gboolean try_connect(gpointer data);
static void server_info_cb(pa_context *c, const pa_server_info *info, void *data);
static void card_info_cb(pa_context *c, const pa_card_info *info, int eol, void *data);
static void sink_info_cb(pa_context *c, const pa_sink_info *info, int eol, void *data);

void pulse_glue_init(void)
{
    loop = pa_glib_mainloop_new(g_main_context_default());
    g_assert(loop);
    api = pa_glib_mainloop_get_api(loop);
    g_assert(api);
}

void pulse_glue_destroy(void)
{
    if (has_postponed_sink_reload)
        g_source_remove(postponed_sink_reload_timeout_id);
    if (sink_reload_operation)
        pa_operation_unref(sink_reload_operation);
    if (context)
        pa_context_unref(context);
    pa_glib_mainloop_free(loop);
}

static gboolean postponed_sink_reload(gpointer data)
{
    // Try again later if another sink reload operation is in progress
    if (sink_reload_operation)
        return TRUE;

    // Start a sink reload operation
    sink_reload_operation = pa_context_get_sink_info_by_index(context,
            default_sink_index, sink_info_cb, NULL);
    if (!sink_reload_operation)
        g_printerr("pa_context_get_sink_info_by_index() failed\n");

    // We no longer have a postponed sink reload operation
    has_postponed_sink_reload = FALSE;

    return FALSE;
}

static void run_or_postpone_sink_reload(void)
{
    // Postpone if a sink reload operation is in progress, do it
    // right away otherwise
    if (sink_reload_operation) {
        if (has_postponed_sink_reload)
            g_source_remove(postponed_sink_reload_timeout_id);
        postponed_sink_reload_timeout_id = g_timeout_add_seconds(1, postponed_sink_reload, NULL);
        has_postponed_sink_reload = TRUE;
    }
    else {
        postponed_sink_reload(NULL);
        return;
    }
}

static void event_cb(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *data)
{
    switch (type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
        case PA_SUBSCRIPTION_EVENT_SERVER:
            {
                // Reload the server info
                pa_operation *oper = pa_context_get_server_info(context, server_info_cb, NULL);
                if (oper)
                    pa_operation_unref(oper);
                else
                    g_printerr("pa_context_get_server_info() failed\n");
            }
            break;
        case PA_SUBSCRIPTION_EVENT_CARD:
            {
                // Ignore this unless we're handling this card
                if (idx != default_card_index)
                    return;

                // Reload the card info
                pa_operation *oper = pa_context_get_card_info_by_index(context,
                        default_card_index, card_info_cb, NULL);
                if (oper)
                    pa_operation_unref(oper);
                else
                    g_printerr("pa_context_get_card_info_by_index() failed\n");
            }
            break;
        case PA_SUBSCRIPTION_EVENT_SINK:
            // If this is the sink we're handling, try to reload the sink status
            if (idx == default_sink_index)
                run_or_postpone_sink_reload();
            break;
        default:
            g_debug("Unhandled subscribed event type");
            break;
    }
}

static void card_info_cb(pa_context *c, const pa_card_info *info, int eol, void *data)
{
    // Check if this is the termination call
    if (eol > 0)
        return;

    // Handle errors
    if (eol < 0 || !info) {
        g_printerr("Sink info callback failure\n");
        return;
    }

    // Add the profiles to the audio status
    audio_status_reset_profiles();
    audio_status *as = shared_audio_status();
    for (uint32_t i = 0; i < info->n_profiles; ++i) {
        pa_card_profile_info *info_profile = &info->profiles[i];
        audio_status_profile *profile = g_malloc(sizeof(audio_status_profile));
        profile->name = g_strdup(info_profile->name);
        profile->description = g_strdup(info_profile->description);
        profile->priority = info_profile->priority;
        profile->active = info->active_profile == info_profile;
        as->profiles = g_slist_append(as->profiles, profile);
    }
    audio_status_sort_profiles();

    // Update the popup menu
    update_popup_menu();
}

static void sink_info_cb(pa_context *c, const pa_sink_info *info, int eol, void *data)
{
    // Check if this is the termination call
    if (eol > 0)
        return;

    // Get rid of the reference to the operation
    g_assert(sink_reload_operation);
    pa_operation_unref(sink_reload_operation);
    sink_reload_operation = NULL;

    // Handle errors
    if (eol < 0 || !info) {
        g_printerr("Sink info callback failure\n");
        return;
    }

    // Check if the default card changed and save it
    gboolean default_card_changed = !have_default_card_index ||
        default_card_index != info->card;
    default_card_index = info->card;
    have_default_card_index = TRUE;

    // Save the default sink and the number of volume channels
    default_sink_index = info->index;
    default_sink_num_channels = info->volume.channels;

    // If we aren't subscribed yet, subscribe now
    if (!subscribed) {
        pa_context_set_subscribe_callback(context, event_cb, NULL);
        pa_operation *oper = pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_SERVER |
                PA_SUBSCRIPTION_MASK_CARD | PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
        if (oper)
            pa_operation_unref(oper);
        else
            g_printerr("pa_context_subscribe() failed\n");
        subscribed = TRUE;
    }

    // Update the audio status
    pa_volume_t volume = pa_cvolume_avg(&(info->volume));
    if (volume > PA_VOLUME_UI_MAX)
        volume = PA_VOLUME_UI_MAX;
    audio_status *as = shared_audio_status();
    as->volume = volume * 150.0 / PA_VOLUME_UI_MAX;
    as->muted = info->mute ? TRUE : FALSE;

    // Update the tray icon and the volume scale
    update_tray_icon();
    update_volume_scale();

    // Start getting information about the card if it changed
    if (default_card_changed) {
        pa_operation *oper = pa_context_get_card_info_by_index(context,
                default_card_index, card_info_cb, NULL);
        if (oper)
            pa_operation_unref(oper);
        else
            g_printerr("pa_context_get_card_info_by_index() failed\n");
    }
}

static void server_info_cb(pa_context *c, const pa_server_info *info, void *data)
{
    // Handle errors
    if (!info) {
        g_printerr("Server info callback failure\n");
        return;
    }

    // Check if we have sinks at all
    if (!info->default_sink_name) {
        g_printerr("No default sink name (don't you have any sinks?)\n");
        gtk_main_quit();
        return;
    }

    // If we have a sync reload operation in progress, get rid of it
    if (has_postponed_sink_reload)
        g_source_remove(postponed_sink_reload_timeout_id);
    if (sink_reload_operation)
        pa_operation_cancel(sink_reload_operation);

    // Get the default sink info
    sink_reload_operation = pa_context_get_sink_info_by_name(context,
            info->default_sink_name, sink_info_cb, NULL);
    if (!sink_reload_operation)
        g_printerr("pa_context_get_sink_info_by_name() failed\n");
    run_or_postpone_sink_reload();
}

static void context_state_cb(pa_context *c, void *data)
{
    // Handle errors
    pa_context_state_t state = pa_context_get_state(context);
    if (state == PA_CONTEXT_FAILED) {
        g_printerr("Failed to connect to the server, retrying soon\n");
        pa_context_unref(context);
        context = NULL;
        g_timeout_add_seconds(1, try_connect, NULL);
        return;
    }

    // Handle the case where the server was terminated
    if (state == PA_CONTEXT_TERMINATED) {
        g_debug("Server terminated, exiting...\n");
        gtk_main_quit();
        return;
    }

    // Now we only handle the ready state
    if (state != PA_CONTEXT_READY)
        return;

    // Start by getting the server information
    pa_operation *oper = pa_context_get_server_info(context, server_info_cb, NULL);
    if (oper)
        pa_operation_unref(oper);
    else
        g_printerr("pa_context_get_server_info() failed\n");
}

static gboolean try_connect(gpointer data)
{
    // Create a new context
    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "pa-applet");
    context = pa_context_new_with_proplist(api, NULL, proplist);
    g_assert(context);
    pa_proplist_free(proplist);

    // Connect the context state callback
    pa_context_set_state_callback(context, context_state_cb, NULL);

    // Try to connect the context
    if (pa_context_connect(context, NULL, PA_CONTEXT_NOFAIL, NULL) < 0) {
        g_printerr("Unable to connect context\n");
        gtk_main_quit();
        return FALSE;
    }

    return FALSE;
}

void pulse_glue_start(void)
{
    try_connect(NULL);
}

void pulse_glue_sync_volume(void)
{
    // Nothing to do if we don't have a context
    if (!context)
        return;

    // Create a volume specification
    audio_status *as = shared_audio_status();
    pa_cvolume volume;
    pa_cvolume_init(&volume);
    pa_cvolume_set(&volume, default_sink_num_channels,
            as->volume * PA_VOLUME_UI_MAX / 150);

    // Set the volume
    pa_operation *oper = pa_context_set_sink_volume_by_index(context,
            default_sink_index, &volume, NULL, NULL);
    if (oper)
        pa_operation_unref(oper);
    else
        g_printerr("pa_context_set_sink_volume_by_index() failed\n");
}

void pulse_glue_sync_muted(void)
{
    // Nothing to do if we don't have a context
    if (!context)
        return;

    // Set the mute switch
    pa_operation *oper = pa_context_set_sink_mute_by_index(context,
            default_sink_index, shared_audio_status()->muted, NULL, NULL);
    if (oper)
        pa_operation_unref(oper);
    else
        g_printerr("pa_context_set_sink_mute_by_index() failed\n");
}

void pulse_glue_sync_active_profile(void)
{
    // Nothing to do if we don't have a context
    if (!context)
        return;

    // Find the active profile
    audio_status_profile *active_profile = NULL;
    audio_status *as = shared_audio_status();
    uint32_t index = 0;
    for (GSList *entry = as->profiles; entry;entry = g_slist_next(entry), ++index) {
        audio_status_profile *profile = (audio_status_profile *)entry->data;
        if (profile->active) {
            active_profile = profile;
            break;
        }
    }
    g_assert(active_profile);

    // Sync with the server
    pa_operation *oper = pa_context_set_card_profile_by_index(context,
            default_card_index, active_profile->name, NULL, NULL);
    if (oper)
        pa_operation_unref(oper);
    else
        g_printerr("pa_context_set_card_profile_by_index() failed\n");
}

/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>

#include "audio_status.h"
#include "popup_menu.h"
#include "pulse_glue.h"
#include "tray_icon.h"
#include "volume_scale.h"

static GtkStatusIcon *tray_icon = NULL;
static gboolean updated_once = FALSE;

static void on_activate(GtkStatusIcon *status_icon, gpointer data)
{
    // Do nothing unless we have been updated at least once
    if (!updated_once)
        return;

    // Hide the volume scale if it's visible
    if (is_volume_scale_visible()) {
        hide_volume_scale();
        return;
    }

    // If we're showing the menu, just hide it
    if (is_popup_menu_visible()) {
        hide_popup_menu();
        return;
    }

    // Show the volume scale
    if (gtk_status_icon_is_embedded(tray_icon)) {
        GdkRectangle rect;
        gtk_status_icon_get_geometry(tray_icon, NULL, &rect, NULL);
        show_volume_scale(&rect);
    }
    else {
        show_volume_scale(NULL);
    }
}

static void on_scroll(GtkStatusIcon *status_icon, GdkEventScroll *event, gpointer data)
{
    // Do nothing unless we have been updated at least once
    if (!updated_once)
        return;

    // Bump the volume level
    switch (event->direction) {
        case GDK_SCROLL_UP:
        case GDK_SCROLL_RIGHT:
            audio_status_raise_volume();
            break;
        case GDK_SCROLL_DOWN:
        case GDK_SCROLL_LEFT:
            audio_status_lower_volume();
            break;
        default:
            return;
    }

    // Sync with the server
    pulse_glue_sync_volume();

    // Inform the user by flashing the volume scale
    update_volume_scale();
    if (gtk_status_icon_is_embedded(tray_icon)) {
        GdkRectangle rect;
        gtk_status_icon_get_geometry(tray_icon, NULL, &rect, NULL);
        flash_volume_scale(&rect);
    }
    else {
        flash_volume_scale(NULL);
    }
}

static void on_menu(GtkStatusIcon *status_icon, gpointer data)
{
    // Do nothing unless we have been updated at least once
    if (!updated_once)
        return;

    // Show the popup menu unless something was already visible
    if (!is_volume_scale_visible() && !is_popup_menu_visible())
        show_popup_menu(tray_icon);
}

static gboolean on_button_release(GtkStatusIcon *status_icon, GdkEventButton *event, gpointer data)
{
    // Do nothing unless we have been updated at least once
    if (!updated_once)
        return FALSE;

    // We only handle releases of the middle mouse button
    if (event->button != 2)
        return FALSE;

    // Update the audio status and sync with the server
    audio_status_toggle_muted();
    pulse_glue_sync_muted();

    // Update the tray icon as well
    update_tray_icon();

    return TRUE;
}

void create_tray_icon(void)
{
    tray_icon = gtk_status_icon_new();
    g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(G_OBJECT(tray_icon), "popup-menu", G_CALLBACK(on_menu), NULL);
    g_signal_connect(G_OBJECT(tray_icon), "scroll_event", G_CALLBACK(on_scroll), NULL);
    g_signal_connect(G_OBJECT(tray_icon), "button-press-event", G_CALLBACK(on_button_release), NULL);
}

void destroy_tray_icon(void)
{
    if (tray_icon) {
        gtk_widget_destroy(GTK_WIDGET(tray_icon));
        tray_icon = NULL;
    }
    destroy_volume_scale();
    destroy_popup_menu();
}

void update_tray_icon(void)
{
    // Yes, we've been updated once now
    updated_once = TRUE;

    // Get the new tray icon name and tooltip text format
    audio_status *as = shared_audio_status();
    gchar *icon_name, *tooltip_text_format;
    if (as->muted) {
        icon_name = "audio-volume-muted";
        tooltip_text_format = "Volume: %d%% (muted)";
    }
    else {
        if (as->volume < 150.0 / 3)
            icon_name = "audio-volume-low";
        else if (as->volume < 150.0 / 3 * 2)
            icon_name = "audio-volume-medium";
        else
            icon_name = "audio-volume-high";
        tooltip_text_format = "Volume: %d%%";
    }

    // Update the icon name
    gtk_status_icon_set_from_icon_name(tray_icon, icon_name);

    // Update the tooltip
    gsize buffer_size = (strlen(tooltip_text_format) + 5) * sizeof(gchar);
    gchar *tooltip_text = g_malloc0(buffer_size);
    if (tooltip_text) {
        g_snprintf(tooltip_text, buffer_size, tooltip_text_format, (int)(as->volume));
        gtk_status_icon_set_tooltip_text(tray_icon, tooltip_text);
        g_free(tooltip_text);
    }

    // Update the volume scale or the popup menu if needed
    if (is_volume_scale_visible())
        update_volume_scale();
    else if (is_popup_menu_visible())
        update_popup_menu();
}

/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#define FLASH_TIMEOUT 750

#include <gtk/gtk.h>

#include "audio_status.h"
#include "pulse_glue.h"
#include "volume_scale.h"

static GtkWidget *window = NULL, *scale;
static gboolean changing_scale_value = FALSE;
static gboolean visible = FALSE, flashing = FALSE;
static guint flashing_timeout_id;

static void on_scale_value_change(GtkRange *range, gpointer data)
{
    // Nothing to do if we changed the scale value programatically
    if (changing_scale_value)
        return;

    // Update the audio volume and sync with the server
    shared_audio_status()->volume = gtk_range_get_value(range);
    pulse_glue_sync_volume();
}

static void create_volume_scale(void)
{
    // Create a popup window
    window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window), 0, 120);

    // Create the scale and add it to the window
    scale = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0, 150.0, 1.0);
    gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
    gtk_range_set_inverted(GTK_RANGE(scale), TRUE);
    gtk_container_add(GTK_CONTAINER(window), scale);
    gtk_widget_show(scale);

    // Connect the value changed signal
    g_signal_connect(G_OBJECT(scale), "value-changed", G_CALLBACK(on_scale_value_change), NULL);
}

void destroy_volume_scale(void)
{
    // Get rid of the popup window
    if (window) {
        gtk_widget_destroy(window);
        window = NULL;
    }
}

static void do_show_volume_scale(GdkRectangle *rect_or_null)
{
    // Create the scale if needed
    if (!window)
        create_volume_scale();

    // Cancel any flashing timeout
    if (flashing)
        g_source_remove(flashing_timeout_id);

    // Update the volume level
    changing_scale_value = TRUE;
    gtk_range_set_value(GTK_RANGE(scale), shared_audio_status()->volume);
    changing_scale_value = FALSE;

    if (rect_or_null) {
        // Determine where the window will be
        gint size_x, size_y;
        gtk_window_get_size(GTK_WINDOW(window), &size_x, &size_y);
        gint x = rect_or_null->x + (rect_or_null->width - size_x) / 2;
        gint y = rect_or_null->y - size_y;

        // Find the coordinates of the monitor
        GdkRectangle monitor_rect;
        GdkScreen *screen = gtk_widget_get_screen(window);
        gint monitor = gdk_screen_get_monitor_at_point(screen, rect_or_null->x, rect_or_null->y);
        gdk_screen_get_monitor_geometry(screen, monitor, &monitor_rect);

        // If the Y position is outside the monitor rect, the tray is at the top
        if (y < monitor_rect.y) {
            y = rect_or_null->y + rect_or_null->height;
        }

        // Position the window
        gtk_window_move(GTK_WINDOW(window), x, y);
    }

    // Show the window
    gtk_window_present(GTK_WINDOW(window));

    // We're visible now, but not flashing
    visible = TRUE;
    flashing = FALSE;
}

static void on_pointer_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    // Hide the volume scale
    hide_volume_scale();

    // Find the pointer device, if possible
    GdkDevice *device = gtk_get_current_event_device();
    if (device && gdk_device_get_source(device) == GDK_SOURCE_KEYBOARD)
        device = gdk_device_get_associated_device(device);
    if (!device) {
        g_printerr("Failed to find the pointer device\n");
        return;
    }

    // Ungrab it
    gdk_device_ungrab(device, GDK_CURRENT_TIME);
    gdk_flush();
}

void show_volume_scale(GdkRectangle *rect_or_null)
{
    // Actually show the volume scale
    do_show_volume_scale(rect_or_null);

    // Find the pointer device, if possible
    GdkDevice *device = gtk_get_current_event_device();
    if (device && gdk_device_get_source(device) == GDK_SOURCE_KEYBOARD)
        device = gdk_device_get_associated_device(device);
    if (!device) {
        g_printerr("Failed to find the pointer device\n");
        return;
    }

    // Grab it so we can hide the scale when the user clicks outside it
    g_signal_connect_after(G_OBJECT(window), "button_press_event", G_CALLBACK(on_pointer_press), NULL);
    gdk_device_grab(device, gtk_widget_get_window(window), GDK_OWNERSHIP_NONE,
            TRUE, GDK_BUTTON_PRESS_MASK, NULL, GDK_CURRENT_TIME);
    gdk_flush();
}

static gboolean on_flash_timeout(gpointer data)
{
    // Hide the window
    gtk_widget_hide(window);

    // No longer visible, no longer flashing
    visible = FALSE;
    flashing = FALSE;

    return FALSE;
}

void flash_volume_scale(GdkRectangle *rect_or_null)
{
    if (visible) {
        // If we're already visible, but not flashing, nothing to do
        if (!flashing)
            return;

        // We're visible and flashing, so keep visible for a bit longer
        g_source_remove(flashing_timeout_id);
        flashing_timeout_id = g_timeout_add(FLASH_TIMEOUT, on_flash_timeout, NULL);
        return;
    }

    // We're not visible, so show the volume scale and set up the timeout
    do_show_volume_scale(rect_or_null);
    flashing_timeout_id = g_timeout_add(FLASH_TIMEOUT, on_flash_timeout, NULL);
    flashing = TRUE;
}

void hide_volume_scale(void)
{
    // Nothing to do if we were hidden anyways
    if (!visible)
        return;

    // Cancel any flashing timeout
    if (flashing)
        g_source_remove(flashing_timeout_id);

    // Hide the window
    gtk_widget_hide(window);

    // No longer visible, no longer flashing
    visible = FALSE;
    flashing = FALSE;
}

gboolean is_volume_scale_visible(void)
{
    return visible && !flashing;
}

void update_volume_scale(void)
{
    // Update the volume level only if we're visible
    if (visible) {
        changing_scale_value = TRUE;
        gtk_range_set_value(GTK_RANGE(scale), shared_audio_status()->volume);
        changing_scale_value = FALSE;
    }
}

/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include <gtk/gtk.h>

#include "audio_status.h"
#include "pulse_glue.h"

void destroy_popup_menu()
{
    // Nothing needs to be done at the moment
}

static void on_item_activate(GtkMenuItem *item, gpointer data)
{
    // If this profile is already active, nothing to do
    audio_status_profile *profile = (audio_status_profile *)data;
    if (profile->active)
        return;

    // Set all other profiles to not active
    audio_status *as = shared_audio_status();
    for (GSList *entry = as->profiles; entry; entry = g_slist_next(entry)) {
        audio_status_profile *profile = (audio_status_profile *)entry->data;
        profile->active = FALSE;
    }

    // Set the selected profile to active
    profile->active = TRUE;
    pulse_glue_sync_active_profile();
}

void show_popup_menu(GtkStatusIcon *status_icon)
{
    // Nothing to do if we have no entries
    audio_status *as = shared_audio_status();
    if (!as->profiles)
        return;

    // Create the menu
    GtkWidget *menu = gtk_menu_new();

    // Populate it
    for (GSList *entry = as->profiles; entry; entry = g_slist_next(entry)) {
        audio_status_profile *profile = (audio_status_profile *)entry->data;
        GtkWidget *item = gtk_check_menu_item_new_with_label(profile->description->str);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), profile->active);
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_item_activate), profile);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }

    // Show it
    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, gtk_status_icon_position_menu,
            status_icon, 0, gtk_get_current_event_time());
}

void hide_popup_menu()
{
    // Nothing needs to be done at the moment
}

gboolean is_popup_menu_visible()
{
    // Right now we can't tell, but it doesn't really matter as
    // if the user managed to click the tray icon, the popup menu
    // lost focus anyways and was thus dismissed
    return FALSE;
}

void update_popup_menu()
{
    // Nothing needs to be done at the moment
}

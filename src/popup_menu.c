/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <string.h>

#include "audio_status.h"
#include "pulse_glue.h"

static GSList *profile_names = NULL;

void destroy_popup_menu()
{
    // Nothing needs to be done at the moment
}

static void on_selection_done(GtkMenu *menu, gpointer data)
{
    // Get rid of the copied profile names
    g_slist_free_full(profile_names, (GDestroyNotify)g_free);
    profile_names = NULL;
}

static void on_item_activate(GtkMenuItem *item, gpointer data)
{
    // Find the corresponding profile
    gchar *profile_name = (gchar *)data;
    audio_status_profile *profile = NULL;
    audio_status *as = shared_audio_status();
    for (GSList *entry = as->profiles; entry; entry = g_slist_next(entry)) {
        audio_status_profile *prof = (audio_status_profile *)entry->data;
        if (!strcmp(prof->name, profile_name)) {
            profile = prof;
            break;
        }
    }

    // It might happen that the profile can't be found, e.g., if the list
    // of profiles has since changed (it's unlikely, but maybe the user
    // changed the default card between showing the menu and now)
    if (!profile) {
        g_debug("The selected profile doesn't exist anymore, ignoring");
        return;
    }

    // If this profile is already active, nothing to do
    if (profile->active)
        return;

    // Set all profiles to not active
    for (GSList *entry = as->profiles; entry; entry = g_slist_next(entry)) {
        audio_status_profile *prof = (audio_status_profile *)entry->data;
        prof->active = FALSE;
    }

    // Set the selected profile to active and sync
    profile->active = TRUE;
    pulse_glue_sync_active_profile();
}

void show_popup_menu(GtkStatusIcon *status_icon)
{
    // Right now we shouldn't have any profile names referenced
    g_assert(!profile_names);

    // Nothing to do if we have no entries
    audio_status *as = shared_audio_status();
    if (!as->profiles)
        return;

    // Create the menu
    GtkWidget *menu = gtk_menu_new();
    g_signal_connect(G_OBJECT(menu), "selection-done", G_CALLBACK(on_selection_done), NULL);

    for (GSList *entry = as->profiles; entry; entry = g_slist_next(entry)) {
        // Create the item
        audio_status_profile *profile = (audio_status_profile *)entry->data;
        GtkWidget *item = gtk_check_menu_item_new_with_label(profile->description);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), profile->active);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

        // Copy and keep a reference to the profile name
        gchar *name_copy = g_strdup(profile->name);
        profile_names = g_slist_prepend(profile_names, name_copy);

        // Connect the signal, referecing the copy of the profile name
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(on_item_activate), name_copy);
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

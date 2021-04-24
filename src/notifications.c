/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include <libnotify/notify.h>

#include "audio_status.h"

#define PROGRAM_NAME "pa-applet"

gboolean have_notifications = FALSE;
NotifyNotification *notification = NULL;

void notifications_init(void)
{
    if (notify_init(PROGRAM_NAME)) {
        // Create and configure the notification
        have_notifications = TRUE;
#if NOTIFY_CHECK_VERSION(0, 7, 0)
        notification = notify_notification_new(PROGRAM_NAME, NULL, NULL);
#else
        notification = notify_notification_new(PROGRAM_NAME, NULL, NULL, NULL);
#endif
        if (notification) {
            notify_notification_set_timeout(notification, NOTIFY_EXPIRES_DEFAULT);
            notify_notification_set_hint_string(notification, "synchronous", "volume");
        }
        else {
            g_printerr("Failed to create a notification\n");
        }
    }
    else {
        g_printerr("Failed to initialize notifications\n");
    }
}

void notifications_destroy(void)
{
    if (have_notifications) {
        notify_uninit();
        if (notification)
            g_object_unref(G_OBJECT(notification));
    }
}

void notifications_flash(void)
{
    // Nothing to do if we don't support notifications
    if (!have_notifications || !notification)
        return;

    // Find the icon name
    const char *icon_name;
    audio_status *as = shared_audio_status();
    if (as->muted) {
        icon_name = "audio-volume-muted";
    }
    else {
        if (as->volume < 150.0 / 3)
            icon_name = "audio-volume-low";
        else if (as->volume < 150.0 / 3 * 2)
            icon_name = "audio-volume-medium";
        else
            icon_name = "audio-volume-high";
    }

    // Update the notification volume
    notify_notification_set_hint_int32(notification, "value", (gint)as->volume);

    // Update the notification icon
    notify_notification_update(notification, PROGRAM_NAME, NULL, icon_name);

    // Show the notification
    notify_notification_show(notification, NULL);
}

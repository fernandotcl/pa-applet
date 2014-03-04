/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include "audio_status.h"
#include "key_grabber.h"
#include "notifications.h"
#include "popsound.h"
#include "pulse_glue.h"
#include "tray_icon.h"

#define KEY_STEP_SIZE 3.0

static void volume_raise_key_pressed(void)
{
    audio_status_raise_volume();
    pulse_glue_sync_volume();
    notifications_flash();
    popsound_play();
}

static void volume_lower_key_pressed(void)
{
    audio_status_lower_volume();
    pulse_glue_sync_volume();
    notifications_flash();
    popsound_play();
}

static void volume_mute_key_pressed(void)
{
    audio_status_toggle_muted();
    pulse_glue_sync_muted();
    notifications_flash();
    popsound_play();
}

static void print_usage(FILE *out)
{
    fprintf(out, "\
Usage: \n\
    pa-applet [--disable-key-grabbing] [--disable-notifications]\n\
    pa-applet --help\n");
}

int main(int argc, char **argv)
{
    struct option long_options[] = {
        { "help", no_argument, 0, 'h' },
        { "disable-key-grabbing", no_argument, 0, 0 },
        { "disable-notifications", no_argument, 0, 0 },
        { NULL, 0, 0, 0 }
    };

    // Parse the command line options
    gboolean key_grabbing_enabled = TRUE, notifications_enabled = TRUE;
    int opt, longindex;
    while ((opt = getopt_long(argc, argv, "c:fhp:s", long_options, &longindex)) != EOF) {
        switch ((char)opt) {
            case 'h':
                print_usage(stdout);
                return EXIT_SUCCESS;
            case 0:
                if (!strcmp(long_options[longindex].name, "disable-key-grabbing")) {
                    key_grabbing_enabled = FALSE;
                    notifications_enabled = FALSE;
                }
                else if (!strcmp(long_options[longindex].name, "disable-notifications")) {
                    notifications_enabled = FALSE;
                }
                break;
            default:
                print_usage(stderr);
                return EXIT_FAILURE;
        }
    }

    // Initialize GTK+
    gtk_init(&argc, &argv);

    // Initialize everything else
    audio_status_init();
    pulse_glue_init();
    create_tray_icon();

    // Enable notifications if we'll use them
    if (notifications_enabled)
        notifications_init();

    popsound_init();

    // Grab the keys if we're configured to grab them
    if (key_grabbing_enabled) {
        key_grabber_register_volume_raise_callback(volume_raise_key_pressed);
        key_grabber_register_volume_lower_callback(volume_lower_key_pressed);
        key_grabber_register_volume_mute_callback(volume_mute_key_pressed);
        key_grabber_grab_keys();
    }

    // Get the Pulse stuff started
    pulse_glue_start();

    // Run the main loop
    gtk_main();

    // Shut everything down
    if (key_grabbing_enabled)
        key_grabber_ungrab_keys();
    if (notifications_enabled)
        notifications_destroy();
    popsound_destroy();
    destroy_tray_icon();
    pulse_glue_destroy();
    audio_status_destroy();

    return EXIT_SUCCESS;
}

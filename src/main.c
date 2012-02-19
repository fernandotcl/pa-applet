/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include <gtk/gtk.h>
#include <stdlib.h>

#include "audio_status.h"
#include "key_grabber.h"
#include "pulse_glue.h"
#include "tray_icon.h"

#define KEY_STEP_SIZE 3.0

static void volume_raise_key_pressed()
{
    audio_status_raise_volume();
    pulse_glue_sync_volume();
}

static void volume_lower_key_pressed()
{
    audio_status_lower_volume();
    pulse_glue_sync_volume();
}

static void volume_mute_key_pressed()
{
    audio_status_toggle_muted();
    pulse_glue_sync_muted();
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    audio_status_init();
    pulse_glue_init();
    create_tray_icon();

    key_grabber_register_volume_raise_callback(volume_raise_key_pressed);
    key_grabber_register_volume_lower_callback(volume_lower_key_pressed);
    key_grabber_register_volume_mute_callback(volume_mute_key_pressed);
    key_grabber_grab_keys();

    pulse_glue_start();

    gtk_main();

    key_grabber_ungrab_keys();
    destroy_tray_icon();
    pulse_glue_destroy();
    audio_status_destroy();

    return EXIT_SUCCESS;
}

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

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    audio_status_init();
    pulse_glue_init();
    create_tray_icon();
    key_grabber_grab_keys();

    pulse_glue_start();

    gtk_main();

    key_grabber_ungrab_keys();
    destroy_tray_icon();
    pulse_glue_destroy();
    audio_status_destroy();

    return EXIT_SUCCESS;
}

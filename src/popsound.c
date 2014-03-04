/*
 * This file is part of pa-applet.
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

/*
Plays a sound with libcanberra when volume is changed with keypress
Event sounds can be configured / disabled in gtk config
See also:
http://0pointer.de/lennart/projects/libcanberra/
*/

#include "audio_status.h"

#include <canberra.h>

ca_context * cacontext;

void popsound_play(void)
{
    audio_status *as = shared_audio_status();

    // play the sound if not muted, and canberra initialized correctly
    if ( (! as->muted) && (cacontext != NULL) ) {
        ca_context_play (cacontext, 1,
            CA_PROP_EVENT_ID, "audio-volume-change",
    	    CA_PROP_EVENT_DESCRIPTION, "volume changed through key press",
            CA_PROP_CANBERRA_CACHE_CONTROL, "permanent",
    	    NULL);
    }
}

void popsound_init(void)
{
    if (ca_context_create (&cacontext) == 0) {
        ca_context_set_driver (cacontext, "pulse");
    } else {
        cacontext = NULL;
        g_printerr("Failed to create libcanbera context\n");
    }
}

void popsound_destroy(void)
{
    ca_context_destroy(cacontext);
}

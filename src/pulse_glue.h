/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#ifndef PULSE_GLUE_H
#define PULSE_GLUE_H

void pulse_glue_init();
void pulse_glue_destroy();
void pulse_glue_start();
void pulse_glue_sync_volume();
void pulse_glue_sync_muted();
void pulse_glue_sync_active_profile();

#endif

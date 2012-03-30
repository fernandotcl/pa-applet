/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#ifndef KEY_GRABBER_H
#define KEY_GRABBER_H

typedef void (*key_grabber_cb)(void);

void key_grabber_grab_keys(void);
void key_grabber_ungrab_keys(void);
void key_grabber_register_volume_raise_callback(key_grabber_cb cb);
void key_grabber_register_volume_lower_callback(key_grabber_cb cb);
void key_grabber_register_volume_mute_callback(key_grabber_cb cb);

#endif

/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#ifndef VOLUME_SCALE_H
#define VOLUME_SCALE_H

#include <gtk/gtk.h>

void destroy_volume_scale(void);
void show_volume_scale(GdkRectangle *rect_or_null);
void flash_volume_scale(GdkRectangle *rect_or_null);
void hide_volume_scale(void);
gboolean is_volume_scale_visible(void);
void update_volume_scale(void);

#endif

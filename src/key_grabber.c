/*
 * This file is part of pa-applet.
 *
 * © 2012 Fernando Tarlá Cardoso Lemos
 *
 * Refer to the LICENSE file for licensing information.
 *
 */

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

#include "key_grabber.h"

#define NUM_KEYS_TO_GRAB 3

static key_grabber_cb volume_raise_cb = NULL;
static key_grabber_cb volume_lower_cb = NULL;
static key_grabber_cb volume_mute_cb = NULL;

static key_grabber_cb *grabbers[NUM_KEYS_TO_GRAB] = {
    &volume_raise_cb,
    &volume_lower_cb,
    &volume_mute_cb
};

static const char *keysym_names[NUM_KEYS_TO_GRAB] = {
    "XF86AudioRaiseVolume",
    "XF86AudioLowerVolume",
    "XF86AudioMute"
};

static KeyCode grabbed_keys[NUM_KEYS_TO_GRAB] = { 0, };

static GdkFilterReturn filter_func(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
    // Skip events other than key presses
    XEvent *xevent = (XEvent *)gdk_xevent;
    if (xevent->type != KeyPress)
        return GDK_FILTER_CONTINUE;

    // Find a match for the key press
    XKeyEvent *keyevent = (XKeyEvent *)xevent;
    for (int i = 0; i < NUM_KEYS_TO_GRAB; ++i) {
        if (keyevent->keycode == grabbed_keys[i]) {
            if (grabbers[i] != NULL)
                (*grabbers[i])();
            return GDK_FILTER_REMOVE;
        }
    }

    // Continue processing this event
    return GDK_FILTER_CONTINUE;
}

void key_grabber_grab_keys()
{
    // Find the X11 display
    GdkDisplay *gdkDisplay = gdk_display_get_default();
    Display *dpy = GDK_DISPLAY_XDISPLAY(gdkDisplay);

    // Resolve the keysym names into keycodes
    for (int i = 0; i < NUM_KEYS_TO_GRAB; ++i) {
        // Resolve the keysym name into a keysym first
        KeySym keysym = XStringToKeysym(keysym_names[i]);
        if (keysym == NoSymbol) {
            g_printerr("Failed to resolve %s into a keysym\n", keysym_names[i]);
            continue;
        }

        // Resolve the keysym into a keycode
        grabbed_keys[i] = XKeysymToKeycode(dpy, keysym);
        if (grabbed_keys[i] == 0) {
            g_printerr("Failed to resolve %s into a keycode\n", keysym_names[i]);
            continue;
        }
    }

    // Grab the keys for all screens
    for (int i = 0; i < gdk_display_get_n_screens(gdkDisplay); ++i) {
        GdkScreen *screen = gdk_display_get_screen(gdkDisplay, i);
        if (screen == NULL)
            continue;

        // Find the X11 root window
        GdkWindow *gdkRoot = gdk_screen_get_root_window(screen);
        Window root = GDK_WINDOW_XID(gdkRoot);

        for (int i = 0; i < NUM_KEYS_TO_GRAB; ++i) {
            // Ignore the keys that we couldn't resolve
            KeyCode keycode = grabbed_keys[i];
            if (keycode == 0)
                continue;

            // Try to grab the keycodes with any modifiers
            gdk_error_trap_push();
            XGrabKey(dpy, keycode, 0, root, True, GrabModeAsync, GrabModeAsync);
            XGrabKey(dpy, keycode, Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);
            XGrabKey(dpy, keycode, Mod5Mask, root, True, GrabModeAsync, GrabModeAsync);
            XGrabKey(dpy, keycode, LockMask, root, True, GrabModeAsync, GrabModeAsync);
            XGrabKey(dpy, keycode, Mod2Mask | Mod5Mask, root, True, GrabModeAsync, GrabModeAsync);
            XGrabKey(dpy, keycode, Mod2Mask | LockMask, root, True, GrabModeAsync, GrabModeAsync);
            XGrabKey(dpy, keycode, Mod5Mask | LockMask, root, True, GrabModeAsync, GrabModeAsync);
            XGrabKey(dpy, keycode, Mod2Mask | Mod5Mask | LockMask, root, True, GrabModeAsync, GrabModeAsync);
            gdk_flush();

            // Handle errors
            if (gdk_error_trap_pop())
                g_printerr("Failed to grab %s\n", keysym_names[i]);
        }

        // Register for X events
        gdk_window_add_filter(gdkRoot, filter_func, NULL);
    }
}

void key_grabber_ungrab_keys()
{
    // Find the X11 display
    GdkDisplay *gdkDisplay = gdk_display_get_default();
    Display *dpy = GDK_DISPLAY_XDISPLAY(gdkDisplay);

    // Ungrab the keys for all screens
    for (int i = 0; i < gdk_display_get_n_screens(gdkDisplay); ++i) {
        GdkScreen *screen = gdk_display_get_screen(gdkDisplay, i);
        if (screen == NULL)
            continue;

        // Find the X11 root window
        GdkWindow *gdkRoot = gdk_screen_get_root_window(screen);
        Window root = GDK_WINDOW_XID(gdkRoot);

        for (int i = 0; i < NUM_KEYS_TO_GRAB; ++i) {
            // Ignore the keys that we couldn't resolve
            KeyCode keycode = grabbed_keys[i];
            if (keycode == 0)
                continue;

            // Ungrab everything
            gdk_error_trap_push();
            XUngrabKey(dpy, keycode, Mod2Mask, root);
            XUngrabKey(dpy, keycode, Mod5Mask, root);
            XUngrabKey(dpy, keycode, LockMask, root);
            XUngrabKey(dpy, keycode, Mod2Mask | Mod5Mask, root);
            XUngrabKey(dpy, keycode, Mod2Mask | LockMask, root);
            XUngrabKey(dpy, keycode, Mod5Mask | LockMask, root);
            XUngrabKey(dpy, keycode, Mod2Mask | Mod5Mask | LockMask, root);
            gdk_flush();
            if (gdk_error_trap_pop())
                g_printerr("Failed to ungrab %s\n", keysym_names[i]);
        }

        // Unregister for X events
        gdk_window_remove_filter(gdkRoot, filter_func, NULL);
    }
}

void key_grabber_register_volume_raise_callback(key_grabber_cb cb)
{
    volume_raise_cb = cb;
}

void key_grabber_register_volume_lower_callback(key_grabber_cb cb)
{
    volume_lower_cb = cb;
}

void key_grabber_register_volume_mute_callback(key_grabber_cb cb)
{
    volume_mute_cb = cb;
}

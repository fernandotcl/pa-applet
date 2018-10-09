bin_PROGRAMS = pa-applet

pa_applet_SOURCES = \
    audio_status.c \
    audio_status.h \
    key_grabber.c \
    key_grabber.h \
    main.c \
    notifications.h \
    notifications.c \
    popup_menu.c \
    popup_menu.h \
    pulse_glue.c \
    pulse_glue.h \
    tray_icon.c \
    tray_icon.h \
    volume_scale.c \
    volume_scale.h

pa_applet_CPPFLAGS = \
    -std=c99 -D_GNU_SOURCE -Wall -Werror -Wno-error=deprecated-declarations \
    $(GLIB_CFLAGS) \
    $(GTK3_CFLAGS) \
    $(LIBPULSE_CFLAGS) \
    $(LIBPULSE_GLIB_CFLAGS) \
    $(LIBNOTIFY_CFLAGS) \
    $(XLIB_CFLAGS)

pa_applet_LDADD = \
    $(GLIB_LIBS) \
    $(GTK3_LIBS) \
    $(LIBPULSE_LIBS) \
    $(LIBPULSE_GLIB_LIBS) \
    $(LIBNOTIFY_LIBS) \
    $(XLIB_LIBS)

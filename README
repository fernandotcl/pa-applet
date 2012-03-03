Introduction
============

pa-applet is a systray-applet that allows you to control some of
PulseAudio's[1] features. More specifically, pa-applet allows you to control
the volume level of the default sink and mute or unmute it. It also allows you
to change the active profile of the default sink, which can be useful to tell
PulseAudio to redirect audio to the HDMI output instead of outputting to the
built-in speakers in a computer connected to an HDMI device.

pa-applet is not meant as a replacement to padevchooser[2] or pavucontrol[3],
which are much more feature-complete projects. pa-applet offers a more limited
feature set, but in a nicer shell, more appropriate for regular desktop usage.

Note that this project is not affiliated with or endorsed by the PulseAudio
project.

References:

[1]: http://www.pulseaudio.org/
[2]: http://0pointer.de/lennart/projects/padevchooser/
[3]: http://freedesktop.org/software/pulseaudio/pavucontrol


Usage
=====

It may sound obvious, but you need to be using PulseAudio in order to use
pa-applet. At the time of this writing, most Linux distributions seem to ship
PulseAudio as the default sound server. Please check your distribution's
support channels if in doubt.

The volume level can be adjusted in the volume slider that pops up when the
tray icon is clicked. Alternatively, you can hover the tray icon with the
mouse pointer and use the mouse "wheel" to adjust the volume level (you can
also use your touchpad or any other configured pointing device).

The volume keys in your keyboard can also be used to tune the volume up or
down. If you have a notification daemon running (such as notify-osd), a
notification should pop up to give you visual feedback on the volume level
being adjusted.

The active profile for the card associated with the default sink can be
changed by clicking the tray icon with the right mouse button. This allows you
to pick a different active profile from a list. If you have multiple audio
ports (such as an HDMI port in a laptop), you can often redirect the audio
output to that port by changing to the right profile.


Configuration
=============

pa-applet can be configured through simple command line arguments. For
information on how pa-applet can be invoked from the command line, please
refer to pa-applet(1).


Future goals
============

We might want to import functionality from padevchooser or even pavucontrol if
it makes sense to do so. One example is the ability to choose the default sink
and source, or to control the volume level of the default source. It could be
interesting to allow control of individual channels, but it's also a UI
challenge.

Other notification and desktop indicator APIs are being developed, and we
should be able to support them as they mature. Wayland support would be nice
as well.

We can also work on translating pa-applet. There are very few strings, so we
should be able to easily support many languages.


Reporting bugs
==============

Please use the GitHub issue tracker to post bug reports. If the issue hasn't
been reported yet and you have a patch, you might skip that step and send me
the patch directly and I'll apply it as soon as possible.


Contributing
============

Any help is appreciated, please feel free to contribute. I'll review your
patches and commit them if they are in accordance with this project's goals,
and you'll be credited for the changes (I may ask you to change a few things
things before your patches are accepted, though).

For the time being, send your patches to:

Fernando Tarlá Cardoso Lemos <fernandotcl AT gmail.com>

Please format your patches with "git format-patch" and then attach them to an
e-mail instead of sending the patches inlined in the message. This makes it
easier for me to apply them.

Another option is to fork the project on GitHub and send me pull requests.

If you want to implement a new feature, please ping me first so that we can
discuss it and also so that we don't end up implementing the same thing twice.


Authors
=======

pa-applet was created by:

Fernando Tarlá Cardoso Lemos <fernandotcl AT gmail.com>

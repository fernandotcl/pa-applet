#
# Regular cron jobs for the pa-applet package
#
0 4	* * *	root	[ -x /usr/bin/pa-applet_maintenance ] && /usr/bin/pa-applet_maintenance

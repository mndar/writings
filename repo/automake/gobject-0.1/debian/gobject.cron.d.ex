#
# Regular cron jobs for the gobject package
#
0 4	* * *	root	[ -x /usr/bin/gobject_maintenance ] && /usr/bin/gobject_maintenance

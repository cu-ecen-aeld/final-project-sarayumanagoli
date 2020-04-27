#!/bin/sh

case "$1" in
	start)
		echo "Starting Socket Server as daemon"
		start-stop-daemon -S -n servertest -a /usr/bin/servertest -- "-d"
		;;
	stop)
		echo "Stopping Socket Server"
		start-stop-daemon -K -n servertest
		;;
	*)
		echo "Usage: $0 {start|stop}"
	exit 1
esac

exit 0

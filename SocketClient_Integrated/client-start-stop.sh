#!/bin/sh

case "$1" in
	start)
		echo "Starting Socket Server as daemon"
		start-stop-daemon -S -n producer -a /usr/bin/producer -- "-d"
		start-stop-daemon -S -n consumer -a /usr/bin/consumer -- "-d"
		;;
	stop)
		echo "Stopping Socket Server"
		start-stop-daemon -K -n producer
		start-stop-daemon -K -n consumer
		;;
	*)
		echo "Usage: $0 {start|stop}"
	exit 1
esac

exit 0

#!/bin/bash
# Tester script for Multithreaded server with periodic timestamp using Netcat
# Note: This script has to be executed only once after running the server
# Author : Steve Kennedy

target=localhost
port=9000
function printusage
{
	echo "Usage: $0 [-t target_ip] [-p port]"
	echo "	Runs a socket test on the aesdsocket application at"
	echo " 	target_ip and port specified by port"
	echo "	target_ip defaults to ${target}" 
	echo "	port defaults to ${port}" 
}

while getopts "t:p:" opt; do
	case ${opt} in
		t )
			target=$OPTARG
			;;
		p )
			port=$OPTARG
			;;
		\? )
			echo "Invalid option $OPTARG" 1>&2
			printusage
			exit 1
			;;
		: )
			echo "Invalid option $OPTARG requires an argument" 1>&2
			printusage
			exit 1
			;;
	esac
done

echo "Testing target ${target} on port ${port}"

# Tests to ensure socket send/receive is working properly on an aesdsocket utility
# running on the system
# @param1 : The string to send
# @param2 : The previous compare file
# @param3 : delay in seconds to test periodic timestamp
# Returns if the test passes, exits with error if the test fails.
function test_send_socket_string
{
	string=$1
	prev_file=$2
	delay_secs=$3
	echo "**** START of TEST CASE ****"
	echo "Test case : Sending string ${string} to server and checks timestamp after a delay of ${delay_secs} seconds"
	
	new_file=`tempfile`
	expected_file=`tempfile`
	echo ${string} | nc ${target} ${port} -w 1 > ${new_file}

	cur_timestamp=$(grep -c "timestamp:" ${new_file})
	echo "No of timestamps currently in server file: ${cur_timestamp}"

	no_of_timestamps_during_delay=$((${delay_secs}/10))
	expected_timestamps=$((${cur_timestamp}+${no_of_timestamps_during_delay}))
	echo "No of timestamps expected after a delay of ${delay_secs} seconds is ${expected_timestamps}"

	sleep ${delay_secs}

	cp ${prev_file} ${expected_file}
	echo ${string} >> ${expected_file}
	entries_expected=$(grep -c ${string} ${expected_file})
	entries_actual=$(grep -c ${string} ${new_file})


	if [ ${entries_expected} != ${entries_actual} ]; then
		echo "Differences found after sending ${string} to ${target} on port ${port}"
		echo "Expected contents to match:"
		cat ${expected_file}
		echo "But found contents:"
		cat ${new_file}
		echo "Test complete with failure"
		exit 1
	else
		echo ${string} | nc ${target} ${port} -w 1 > ${new_file}
		verify_timestamps=$(grep -c "timestamp:" ${new_file})
		echo "No of timestamps found in file: ${verify_timestamps}"

		if [ ${verify_timestamps} -ge ${expected_timestamps} ]; then
			cp ${expected_file} ${prev_file}
			rm ${new_file}
			rm ${expected_file}
			echo "**** END OF TEST CASE ****"
		else
			echo "Timestamp count doesn't match"
			exit 1
		fi

		
	fi
}

comparefile=`tempfile`
test_send_socket_string "abcdefg" ${comparefile} 10
test_send_socket_string "hijklmnop" ${comparefile} 20
test_send_socket_string "1234567890" ${comparefile} 40
test_send_socket_string "9876543210" ${comparefile} 50
echo "Tests complete with success, last response from server was"
cat ${comparefile}
rm ${comparefile}

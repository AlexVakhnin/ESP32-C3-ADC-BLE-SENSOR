#!/usr/bin/expect -f
#
#  Change the line "set dev" to your mac address !
#
#  Usage with crontab:
#  * * * * * /root/ble.sh>/dev/null
#  Log file:
#  /root/blelog.yymmdd
#
set dev "34:B7:DA:F8:4C:B2"
set uuid "d8182a40-7316-4cbf-9c6e-be507a76d775"
set timeout 5
set thold 12
set now [timestamp -format {%Y-%m-%d %H:%M:%S}]
set logpref [timestamp -format {%y%m%d}]
set CTRL_C     \x03
log_file "/root/blelog.$logpref"

log_user 0
spawn bluetoothctl
expect {
  "Waiting to connect to bluetoothd..." { exp_continue }
  "Agent registered" { expect "# " }
  timeout {
    log_user 1
    send_user "$now bluetooth daemon not found..\n"
	log_user 0
    send $CTRL_C
    expect eof
    exit
  }
}
send -- "remove $dev\r"
expect -re "Device has been removed|Device $dev not available"
expect "# "
send -- "scan on\r"
expect "Discovery started"
sleep 2
expect "Device $dev"
expect "# "
send -- "scan off\r"
expect "# "
send -- "connect $dev\r"
expect {
	"Connection successful" {expect "$uuid"}
	"Device $dev not available" {
		expect "# "
		log_user 1
		send_user "$now Sensor not found..\n"
		log_user 0
		send -- "exit\r"
		expect eof
		exit
	}
}
expect "# "
send -- "gatt.select-attribute $uuid\r"
expect "# "
send -- "gatt.write \"97 116 118\"\r" 
expect -re "Attempting to write|Device $dev not available"
expect "# "
send -- "gatt.read\r"
expect "Value:"
expect "0d 0a"
expect ".."
set str $expect_out(buffer)
log_user 1
#puts "Buffer is: <^$str^>"
set voltage [string trim $str " ."]
if {$voltage < 9 || $voltage > 15} {
	send_user "$now <^$voltage^>  ERROR VOLTAGE..\n"
	log_user 0
	expect "# "
	send -- "disconnect $dev\r"
	expect "Attempting to disconnect"	
} elseif {$voltage > $thold} {
	send_user "$now $voltage > $thold  OK!\n"
	log_user 0
	expect ".."
	expect "# "
	send -- "disconnect $dev\r"
	expect "Attempting to disconnect"
} else {
	send_user "$now $voltage <= $thold  Linux shutdown..\n"
	log_user 0
	expect "# "
	send -- "gatt.write \"115 104 117 116 100 111 119 110\"\r" 
	expect -re "Attempting to write|Device $dev not available"
	expect "# "
	send -- "disconnect $dev\r"
	expect "Attempting to disconnect"
	exec /sbin/shutdown -h now
}
expect "# "
send -- "exit\r"
expect eof

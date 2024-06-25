#!/usr/bin/expect -f
#
#  Usage with crontab:
#  * * * * * /root/ble.sh>>/root/blelog.txt
#
#
set dev "EC:DA:3B:BE:25:16"
set uuid "d8182a40-7316-4cbf-9c6e-be507a76d775"
set timeout 5
set thold 5.1
set now [timestamp -format {%Y-%m-%d %H:%M:%S}]

log_user 0
spawn bluetoothctl
expect "Agent registered"
expect "#"
send -- "remove $dev\r"
expect -re "Device has been removed|Device $dev not available"
expect "#"
send -- "scan on\r"
expect "Discovery started"
sleep 2
expect "Device $dev"
expect "#"
send -- "scan off\r"
expect "#"
send -- "pair $dev\r"
expect -re "Attempting to pair|Device $dev not available"
expect "#"
send -- "connect $dev\r"
expect {
	"Connection successful" {expect "$uuid"}
	"Device $dev not available" {
		expect "#"
		log_user 1
		puts "$now Sensor not found.."
		log_user 0
		send -- "exit\r"
		expect eof
		exit
	}
}
expect "#"
send -- "gatt.select-attribute $uuid\r"
expect "#"
send -- "gatt.write \"97 116 118\"\r" 
expect -re "Attempting to write|Device $dev not available"
expect "#"
send -- "gatt.read\r"
expect "Value:"
expect "0d 0a"
expect ".."
set str $expect_out(buffer)
expect "#"
send -- "disconnect $dev\r"
expect "Attempting to disconnect"
expect "#"
log_user 1
#puts "Buffer is: <^$str^>"
set voltage [string trim $str " ."]
if {$voltage > $thold} {
	puts "$now $voltage > $thold  OK!"
} else {
	puts "$now $voltage <= $thold  BAD.."
#	exec /sbin/shutdown -h now
}
log_user 0
send -- "exit\r"
expect eof

#!/usr/bin/expect -f
set dev "EC:DA:3B:BE:25:16"
set uuid "d8182a40-7316-4cbf-9c6e-be507a76d775"
set timeout 5
set thold 5.101

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
#expect "                             "
expect "0d 0a"
expect ".."
set str $expect_out(buffer)
expect "#"
send -- "disconnect $dev\r"
expect "Attempting to disconnect"
expect "#"
puts "Buffer is: <^$str^>"
set lst [split $str "."]
set voltage [lindex $lst 0].[lindex $lst 1]
if {$voltage > $thold} {
	puts "$voltage > $thold"
	puts "OK!"
} else {
	puts "$voltage <= $thold"
	puts "BAD.."
#	exec /sbin/shutdown -h now
}
send -- "exit\r"
expect eof

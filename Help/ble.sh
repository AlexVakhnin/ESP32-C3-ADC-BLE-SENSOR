#!/usr/bin/expect -f
set dev "EC:DA:3B:BE:25:16"
set uuid "d8182a40-7316-4cbf-9c6e-be507a76d775"
set timeout 30
set thold 5.098

spawn bluetoothctl
expect "Agent registered"
expect "#"
send -- "remove $dev\r"
expect -re "Device has been removed|Device $dev not available"
expect "#"
sleep 2
send -- "scan on\r"
expect "Discovery started"
sleep 2
expect "Device $dev"
expect "#"
send -- "scan off\r"
expect "#"
send -- "pair $dev\r"
expect "Attempting to pair"
expect "#"
send -- "connect $dev\r"
expect "Connection successful"
expect "$uuid"
expect "#"
send "gatt.select-attribute $uuid\r"
expect "#"
send "gatt.write \"97 116 118\"\r" 
expect "#"
send "gatt.read\r"
expect "Value:"
expect "                             "
expect ".."
set str $expect_out(buffer)
set lst [split $str "."]
set voltage [lindex $lst 0].[lindex $lst 1]
puts "The Voltage is: <$voltage>"
if {$voltage > $thold} {
	puts "$voltage > $thold"
	puts "OK!"
} else {
	puts "$voltage <= $thold"
	puts "BAD.."
	exec /sbin/shutdown -h now
}
expect "#"
send -- "exit\r"
expect eof

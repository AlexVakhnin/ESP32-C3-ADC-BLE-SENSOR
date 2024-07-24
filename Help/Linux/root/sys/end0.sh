#! /usr/bin/bash
iptables -F
iptables -t nat -F
iptables-restore < /root/sys/rules.v4.end0
iptables -S
iptables -t nat -S
iptables-save > /etc/iptables/rules.v4


#! /usr/bin/bash
iptables -F
iptables -t nat -F
iptables-restore < /root/sys/rules.v4.eth1
iptables -S
iptables -t nat -S
iptables-save > /etc/iptables/rules.v4

iptables -A INPUT -p tcp -m tcp --dport 22 -m state --state NEW -m recent --set --name SSH --rsource
iptables -A INPUT -p tcp -m tcp --dport 22 -m recent --rcheck --seconds 330 --hitcount 4 --rttl --name SSH --rsource -j REJECT --reject-with tcp-reset
iptables -A INPUT -p tcp -m tcp --dport 22 -m recent --rcheck --seconds 330 --hitcount 3 --rttl --name SSH --rsource -j LOG --log-prefix "SSH brute force "
iptables -A INPUT -p tcp -m tcp --dport 22 -m recent --update --seconds 330 --hitcount 3 --rttl --name SSH --rsource -j REJECT --reject-with tcp-reset
iptables -A INPUT -p tcp -m tcp --dport 22 -j ACCEPT

#based on https://rudd-o.com/linux-and-free-software/a-better-way-to-block-brute-force-attacks-on-your-ssh-server

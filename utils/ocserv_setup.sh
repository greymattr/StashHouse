#!/bin/bash
#
# script to try to automate ocserv setup on a device


sudo apt-get -y install ocserv
sudo apt-get -y install gnutls-bin

HERE="`pwd`"

cd /etc/ocserv
# sudo vi ca.tmpl
echo "cn = \"VPN Test CA\"" | sudo tee ca.tmpl
echo "organization = \"TEST VPN SERVER\"" | sudo tee -a ca.tmpl
echo "serial = 1" | sudo tee -a ca.tmpl
echo "expiration_days = 3650" | sudo tee -a ca.tmpl
echo "ca" | sudo tee -a ca.tmpl
echo "signing_key" | sudo tee -a ca.tmpl
echo "cert_signing_key" | sudo tee -a ca.tmpl
echo "crl_signing_key" | sudo tee -a ca.tmpl


sudo certtool --generate-privkey --outfile ca-key.pem
sudo certtool --generate-self-signed --load-privkey ca-key.pem --template ca.tmpl --outfile ca-cert.pem

INET_ADDR_TEST="`sudo ifconfig | grep -m 1 'inet addr:' | cut -d':' -f2 | cut -d' ' -f1`"

  
echo "cn = \"192.168.1.13\"" | sudo tee server.tmpl
echo "organization = \"TEST VPN SERVER\"" | sudo tee -a server.tmpl
echo "expiration_days = 3650" | sudo tee -a server.tmpl
echo "signing_key" | sudo tee -a server.tmpl
echo "encryption_key" | sudo tee -a server.tmpl
echo "tls_www_server" | sudo tee -a server.tmpl
	
sudo certtool --generate-privkey --outfile server-key.pem
sudo certtool --generate-certificate --load-privkey server-key.pem --load-ca-certificate ca-cert.pem --load-ca-privkey ca-key.pem --template server.tmpl --outfile server-cert.pem

# sudo vim ocserv.conf

sudo sed -i "s|pam\[gid-min=1000\]|plain[/etc/ocserv/ocpasswd]|g" ocserv.conf
sudo sed -i "s|server-cert = /etc/ssl/certs/ssl-cert-snakeoil.pem|server-cert = /etc/ocserv/server-cert.pem|g" ocserv.conf
sudo sed -i "s|server-key = /etc/ssl/private/ssl-cert-snakeoil.key|server-key = /etc/ocserv/server-key.pem|g" ocserv.conf
sudo sed -i "s/try-mtu-discovery = false/try-mtu-discovery = true/g" ocserv.conf
sudo sed -i "s/dns = 192.168.1.2/dns = 8.8.8.8/g" ocserv.conf
sudo sed -i "s/tcp-port = 443/tcp-port = 1443/g" ocserv.conf

sudo sed -i "s|route = 10.0.0.0/8|#route = 10.0.0.0/8|g" ocserv.conf
sudo sed -i "s|route = 192.168.0.0/16/255.255.0.0|#route = 192.168.0.0/16|g" ocserv.conf
sudo sed -i "s|no-route = 192.168.5.0/255.255.255.0|#no-route = 192.168.5.0/255.255.255.0|g" ocserv.conf

echo "enter VPN password for user $USER"	
sudo ocpasswd -c /etc/ocserv/ocpasswd $USER


# sudo vim /etc/sysctl.conf
sudo sed -i "s/#net.ipv4.ip_forward=1/net.ipv4.ip_forward=1/g" /etc/sysctl.conf
	
sudo sysctl -p

sudo iptables -A INPUT -p tcp --dport 1443 -j ACCEPT
sudo iptables -A INPUT -p udp --dport 1443 -j ACCEPT
sudo iptables -t nat -A POSTROUTING -j MASQUERADE

# sudo dpkg-reconfigure iptables-persistent
#( to save iptables rules )

sudo systemctl stop ocserv.socket
#( stop the service)

sudo ocserv -c /etc/ocserv/ocserv.conf
#( start the service )



#EXTRA:

#expose certificate

# sudo iptables -A INPUT -p tcp –dport 80 -j ACCEPT
# sudo apt-get install apache2
# sudo cp /etc/ocserv/ca-cert.pem /var/www/html

cd $HERE

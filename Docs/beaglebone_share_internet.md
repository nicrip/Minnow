# BlueRobotics Share Internet from Connected Computer

On Beaglebone:

```sh
sudo /sbin/route add default gw 192.168.7.1
```
edit /etc/resolv.conf and add the following line at the end:
```sh
nameserver 8.8.8.8
```

On computer (with ip 192.168.7.1 to BBB, connected to internet)

```sh
sudo iptables -A POSTROUTING -t nat -j MASQUERADE
sudo echo 1 |sudo tee /proc/sys/net/ipv4/ip_forward > /dev/null
```

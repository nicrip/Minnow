# Getting Wifi on the Beaglebone Black

We use the following USB wifi stick which is plug and play on Debian:  

TP-Link TL-WN722N (https://www.amazon.com/TP-Link-TL-WN722N-Wireless-network-Adapter/dp/B002SZEOLG/ref=sxin_0_ac_d_pm?ac_md=1-0-VW5kZXIgJDIw-ac_d_pm&cv_ct_cx=TL-WN722N&keywords=TL-WN722N&pd_rd_i=B002SZEOLG&pd_rd_r=efd8e9d5-1f52-47e9-805a-e4ce41777cca&pd_rd_w=ADBMf&pd_rd_wg=PFZSU&pf_rd_p=b8b03b37-bd30-4468-adff-11c42ccb6582&pf_rd_r=TT52RP0SNY2QBHXWG0FB&psc=1&qid=1579799542&s=hi&smid=ATVPDKIKX0DER&sr=1-1-22d05c05-1231-4126-b7c4-3e7a9c0027d0)

When working, a green LED will blink on the stick, and iwconfig will show the adapter.  

To have the Beaglebone automatically reconnect to wifi, we need to setup a script. First, login as superuser:

```
> sudo su
```

Then delete any wifi management programs, such as wpa_supplicant and network-manager:

```
> sudo apt-get purge network-manager
> sudo apt-get remove wpasupplicant
```

Then, make a script called "wifi" in /usr/local/bin with the following:

```
#!/bin/bash    

# make sure we aren't running already
what=`basename $0`
for p in `ps h -o pid -C $what`; do
        if [ $p != $$ ]; then
                exit 0
        fi
done

# source configuration
. /etc/wifi.conf
exec 1> /dev/null
exec 2>> $log
echo $(date) > $log
# without check_interval set, we risk a 0 sleep = busy loop
if [ ! "$check_interval" ]; then
        echo "No check interval set!" >> $log
        exit 1
fi

startWifi () {
        ifconfig $wlan down
        dhclient -v -r
    # make really sure
        killall dhclient
        ifconfig $wlan up
        iwconfig $wlan essid $essid
        dhclient -v $wlan
}

ifconfig $wlan up
startWifi

while [ 1 ]; do
        ping -c 1 $router_ip & wait $!
        if [ $? != 0 ]; then
                echo $(date)" attempting restart..." >> $log
                startWifi
                sleep 1
        else sleep $check_interval
        fi
done
```

Make this script executable using:

```
> chmod u+x /usr/local/bin/wifi
```

This script uses a config file at /etc/wifi.conf:

```
router_ip=192.168.1.1
log=/var/log/wifi.log
wlan=wlan0
essid=Silvertail
check_interval=5
```

Then this script must be able to start when the Beaglebone boots up. This can be done by making a script in /etc/rc.local:

```
#!/bin/sh -e
#
# rc.local
#

echo "Starting wifi..."
/usr/bin/nice -n -10 /usr/local/bin/wifi &

exit 0
```

Make this script executable using:

```
> chmod u+x /etc/rc.local
```

And reboot - the script should run automatically, and be visibile as "wifi" when you run ps -A !!!!

If there are issues, you may need to check that the rc-local service is running. Use:

```
> systemctl status rc-local
```

and inspect the output. If the service doesn't exist, you can start it using:

```
> systemctl enable rc-local
> systemctl start rc-local.service
```

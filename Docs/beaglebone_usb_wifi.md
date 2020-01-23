# Getting Wifi on the Beaglebone Black

We use the following USB wifi stick which is plug and play on Debian:  

TP-Link TL-WN722N (https://www.amazon.com/TP-Link-TL-WN722N-Wireless-network-Adapter/dp/B002SZEOLG/ref=sxin_0_ac_d_pm?ac_md=1-0-VW5kZXIgJDIw-ac_d_pm&cv_ct_cx=TL-WN722N&keywords=TL-WN722N&pd_rd_i=B002SZEOLG&pd_rd_r=efd8e9d5-1f52-47e9-805a-e4ce41777cca&pd_rd_w=ADBMf&pd_rd_wg=PFZSU&pf_rd_p=b8b03b37-bd30-4468-adff-11c42ccb6582&pf_rd_r=TT52RP0SNY2QBHXWG0FB&psc=1&qid=1579799542&s=hi&smid=ATVPDKIKX0DER&sr=1-1-22d05c05-1231-4126-b7c4-3e7a9c0027d0)

When working, a green LED will blink on the stick, and iwconfig will show the adapter.  

Install network-manager using:  

```
> sudo apt-get install network-manager
```

And you can list available wifi connections using:

```
> nmcli dev wifi
```

And connect to a specific ESSID using:

```
> sudo nmcli device wifi connect "MIT SECURE" password "pass"
```

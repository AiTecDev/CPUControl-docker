#!/bin/sh

FILE="/etc/systemd/timesyncd.conf"
echo bash -c "echo 'NTP=0.arch.pool.ntp.org 1.arch.pool.ntp.org 2.arch.pool.ntp.org 3.arch.pool.ntp.org' >> $FILE"
echo bash -c "echo 'FallbackNTP=0.pool.ntp.org 1.pool.ntp.org 0.us.pool.ntp.org' >> $FILE"
cat $FILE
echo systemctl restart systemd-timesyncd.service

# fix USB device mode
DIR="/opt/nvidia/l4t-usb-device-mode"
echo cp $DIR/nv-l4t-usb-device-mode-start.sh $DIR/nv-l4t-usb-device-mode.sh-start.orig
echo cp $DIR/nv-l4t-usb-device-mode-stop.sh $DIR/nv-l4t-usb-device-mode-stop.sh.orig
cat $DIR/nv-l4t-usb-device-mode-start.sh | grep dhcpd_.*=
cat $DIR/nv-l4t-usb-device-mode-stop.sh | grep dhcpd_.*=
echo sed -i 's|${script_dir}/dhcpd.leases|/run/dhcpd.leases|g' $DIR/nv-l4t-usb-device-mode-start.sh
echo sed -i 's|${script_dir}/dhcpd.pid|/run/dhcpd.pid|g' $DIR/nv-l4t-usb-device-mode-start.sh
echo sed -i 's|${script_dir}/dhcpd.leases|/run/dhcpd.leases|g' $DIR/nv-l4t-usb-device-mode-stop.sh
echo sed -i 's|${script_dir}/dhcpd.pid|/run/dhcpd.pid|g' $DIR/nv-l4t-usb-device-mode-stop.sh
cat $DIR/nv-l4t-usb-device-mode-start.sh | grep dhcpd_.*=
cat $DIR/nv-l4t-usb-device-mode-stop.sh | grep dhcpd_.*=

# enable i2c permissions
echo usermod -aG i2c $USER

# install pip and some apt dependencies
echo apt-get update
echo apt-get install -y python3-pip python3-pil python3-smbus python3-matplotlib cmake
#echo python3 -m pip install -U pip
#echo python3 -m pip install flask
#echo python3 -m pip install -U --upgrade numpy

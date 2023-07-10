#!/bin/sh
echo "Installing kernel module ..."
insmod ./kernel/infectivity_kernel.ko
if [ ! $? -eq 0 ]; then
    echo "Failed to insert kernel module"
    exit 1
fi
dhcp_file=/tmp/dhcp.leases

echo > .infect_file
cd user
chmod 755 infectivity_monitor.infec

./infectivity_monitor.infec -start ../.infect_file 192.168.1.0 255.255.255.0 192.168.1.1
if [ ! $? -eq 0 ]; then
    echo "Infectivity Monitor offline"
    ./infectivity_monitor.infec -stop 
    exit 1
fi
echo "Infectivity Monitor online"

chmod 755 connectivity_check.infec
./connectivity_check.infec -start /tmp/dhcp.leases 
if [ ! $? -eq 0 ]; then
    echo "Connectivity Checker offline"
    ./connectivity_check.infec -stop
    exit 1
fi
echo "Connectivity Checker online"
cd ..

LUCI_DIR=/usr/lib/lua/luci/
LUCI_DIR_CONTROLLER=$LUCI_DIR/controller
LUCI_DIR_VIEW=$LUCI_DIR/view

[ -d "$LUCI_DIR_CONTROLLER/infectivity_controller" ] && rm -r $LUCI_DIR_CONTROLLER/infectivity_controller
cp -r ./luci/infectivity_controller $LUCI_DIR_CONTROLLER
echo "LuCI Controller in place"

[ -d "$LUCI_DIR_VIEW/infectivity_view" ] && rm -r $LUCI_DIR_VIEW/infectivity_view
cp -r ./luci/infectivity_view $LUCI_DIR_VIEW
echo "LuCI View in place"

rm -rf /tmp/luci-modulecache/*
/etc/init.d/uhttpd restart
echo "LuCI cache emptied"



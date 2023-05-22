#!/bin/sh
LUCI_DIR=/usr/lib/lua/luci/
LUCI_DIR_CONTROLLER=$LUCI_DIR/controller
LUCI_DIR_VIEW=$LUCI_DIR/view

[ -d "$LUCI_DIR_VIEW/infectivity_view" ] && rm -r $LUCI_DIR_VIEW/infectivity_view
[ -d "$LUCI_DIR_CONTROLLER/infectivity_controller" ] && rm -r $LUCI_DIR_CONTROLLER/infectivity_controller
echo "Module for LuCI removed"

cd user
./connectivity_check.infec -stop
echo "Connectivity Checker offline"

./infectivity_monitor.infec -stop 
echo "Infectivity Monitor offline"

cd ..

# rmmod infectivity_kernel
# echo "Kernel Module removed"

# echo "Initiating reboot ..."
# reboot
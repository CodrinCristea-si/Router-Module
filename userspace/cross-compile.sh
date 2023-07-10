#!/bin/bash
connect_name=connectivity-check
monitor_name=infectivity-monitor
updater_name=updater
tester_name=tester
if [ ! -z "$1" ] ; then 
    connect_name=$1 
fi
if [ ! -z "$2" ] ; then 
    monitor_name=$2 
fi
if [ ! -z "$3" ] ; then 
    updater_name=$3 
fi
if [ ! -z "$4" ] ; then 
    tester_name=$4 
fi
#spice_folder=/run/user/1000/gvfs/dav+sd:host=Spice%2520client%2520folder._webdav._tcp.local/dropout
#sendof_path=/run/user/1000/gvfs/dav+sd:host=Spice%2520client%2520folder._webdav._tcp.local/dropout/sendof.sh
source ../../../openwrt_build/openwrt/bin/openwrt.config
make -f Makefile-cross CONNECT_NAME=$connect_name MONITOR_NAME=$monitor_name UPDATER_NAME=$updater_name TESTER_NAME=$tester_name
if [ $? -eq 0 ]; 
then
    echo "Compilation Success"
    exit 0
else
    echo "Compilation Failed"
    exit 1
fi
#cp $output_file $spice_folder
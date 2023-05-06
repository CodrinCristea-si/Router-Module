#!/bin/bash
output_name=test-connect-neig
if [ ! -z "$1" ] ; then 
    output_name=$1 
fi
spice_folder=/run/user/1000/gvfs/dav+sd:host=Spice%2520client%2520folder._webdav._tcp.local/dropout
sendof_path=/run/user/1000/gvfs/dav+sd:host=Spice%2520client%2520folder._webdav._tcp.local/dropout/sendof.sh
source ../../../openwrt_build/openwrt/bin/openwrt.config
make -f Makefile-cross OUTPUT_NAME=$output_name
cp $output_name $spice_folder
#$sendof_path $output_name

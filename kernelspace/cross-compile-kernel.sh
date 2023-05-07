#!/bin/bash
output_name=infectivity-test
# if [ ! -z "$1" ] ; then 
#     output_name=$1 
# fi
spice_folder=/run/user/1000/gvfs/dav+sd:host=Spice%2520client%2520folder._webdav._tcp.local/dropout
sendof_path=/run/user/1000/gvfs/dav+sd:host=Spice%2520client%2520folder._webdav._tcp.local/dropout/sendof.sh
#source ../../../openwrt_build/openwrt/bin/openwrt.config
#make -f Makefile-cross OUTPUT_NAME=$output_name
rm -r  /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/common
mkdir /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/common
cp -r ../common/*.h /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/common

rm -r  /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/headers
mkdir /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/headers
cp -r ./headers/*.h /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/headers


rm -r /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/source
mkdir /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/source
cp -r ./source/*.c /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src/source


old_dir=$(pwd)
cd /home/babushka19/Desktop/openwrt_build/openwrt/package/kernel/infectivity-test/src
tree -if . |grep -E ".*\.c"|sed -e "s/\.c/\.o/" > a.out
ext_files=$( cat a.out | tr '\n' ' ')
rm a.out
echo "MODULE_NAME := $output_name" > Makefile
echo "obj-m += $output_name.o" >> Makefile
echo "$output_name-objs=$ext_files" >> Makefile
cd /home/babushka19/Desktop/openwrt_build/openwrt
make package/infectivity-test/{clean,compile} KERNEL_PATCHVER=5.10 V=s
output_file=$output_name.ko
cp /home/babushka19/Desktop/openwrt_build/openwrt/build_dir/target-mipsel_24kc_musl/linux-ramips_mt76x8/infectivity-test/$output_file $old_dir
cd $old_dir
cp $output_file $spice_folder
#$sendof_path $output_name
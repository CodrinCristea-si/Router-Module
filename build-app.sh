#!/bin/bash
install_dir="./infectivity_router_module"
[ ! -d "$install_dir" ] && mkdir $install_dir
echo "Install directory available"

old_dir=$(pwd)

module_kernel_name=infectivity_kernel
module_kernel_file=$module_kernel_name.ko
kernel_dir=./kernelspace

cd $kernel_dir
echo "Cross compilling kernel module ..."
./cross-compile-kernel.sh $module_kernel_name
if [ ! $? -eq 0 ]; 
then 
    exit 1 
fi

cd $old_dir

[ ! -d "$install_dir/kernel" ] && mkdir $install_dir/kernel
cp $kernel_dir/$module_kernel_file $install_dir/kernel
echo "Module $module_kernel_file moved to $install_dir/kernel"

connect_name=connectivity_check.infec
monitor_name=infectivity_monitor.infec
updater_name=updater.infec
userspace_dir=./userspace

cd $userspace_dir
echo "Cross compilling userspace apps  ..."
./cross-compile.sh $connect_name $monitor_name $updater_name
if [ ! $? -eq 0 ]; 
then 
    exit 1  
fi
cd $old_dir

[ ! -d "$install_dir/user" ] && mkdir $install_dir/user
echo "User apps directory available"
cp $userspace_dir/$connect_name $install_dir/user
echo "Module $connect_name moved to $install_dir/user"
cp $userspace_dir/$monitor_name $install_dir/user
echo "Module $monitor_name moved to $install_dir/user"
cp $userspace_dir/$updater_name $install_dir/user
echo "Module $updater_name moved to $install_dir/user"

luci_dir=./luci_interface
[ ! -d "$install_dir/luci" ] && mkdir $install_dir/luci
echo "LuCI directory available"

cd $luci_dir
cp -r ./infectivity_controller $old_dir/$install_dir/luci
echo "Module infectivity_controller moved to $install_dir/luci"

cp -r ./infectivity_view $old_dir/$install_dir/luci
echo "Module infectivity_view moved to $install_dir/luci"

cd $old_dir
cp ./start-infectivity.sh $install_dir
echo "Installer moved to $install_dir"

cp ./stop-infectivity.sh $install_dir
echo "Uninstaller moved to $install_dir"

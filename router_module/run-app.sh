./kill-app.sh
cd kernelspace/
./local-compile-kernel.sh
insmod infectivity-test.ko
cd ..
cd userspace/
./local-compile.sh 
./connect-test.test -start /home/babushka19/Desktop/openwrt-network-module/module_dev/userspace/dhcp-probs.txt
valgrind ./get_update.test
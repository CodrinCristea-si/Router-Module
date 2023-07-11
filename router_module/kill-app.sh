cd kernelspace/
rmmod infectivity-test.ko
cd ..
cd userspace/
./connect-test.test -stop
cd ..
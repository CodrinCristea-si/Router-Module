mkdir ./common
cp -r ../common/* ./common
make V=s
rm -r ./common
../clear-leftover.sh
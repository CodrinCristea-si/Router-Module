mkdir ./common
cp -r ../common/* ./common
make V=s
rm -r ./common
cd ..
./clear-leftover.sh
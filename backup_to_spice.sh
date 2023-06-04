
spice_folder=/run/user/1000/gvfs/dav+sd:host=Spice%2520client%2520folder._webdav._tcp.local/dropout
echo "hai sa stergem"
[ -d "$spice_folder/common" ] && rm -r $spice_folder/common/*
echo "common removed"
cp -r ./common/* $spice_folder
echo "common copied"

[ -d "$spice_folder/kernelspace" ] && rm -r $spice_folder/kernelspace/*
echo "kernelspace removed"
cp -r ./kernelspace/* $spice_folder
echo "kernelspace copied"

[ -d "$spice_folder/userspace" ] && rm -r $spice_folder/userspace/*
echo "userspace removed"
cp -r ./userspace/* $spice_folder
echo "userspace copied"
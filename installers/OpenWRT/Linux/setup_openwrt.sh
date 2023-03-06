rem This is an automatic installer for OpenWRT OS for Linux Distributed Systems.
rem This installer must run with administrative privileges (sudo) and with the recommanded mask of 755 
OS_FILE=openwrt_file.img.gz
IMG_FILE=openwrt_file.img
OS_URL=https://downloads.openwrt.org/releases/22.03.3/targets/x86/64/openwrt-22.03.3-x86-64-generic-ext4-combined.img.gz
rem OS_URL=https://downloads.openwrt.org/releases/22.03.3/targets/x86/64/openwrt-22.03.3-x86-64-generic-ext4-combined-efi.img.gz
rem OS_URL=https://downloads.openwrt.org/releases/22.03.3/targets/x86/64/openwrt-22.03.3-x86-64-generic-squashfs-combined.img.gz
echo "Downloading openwrt image ..."
wget --output-document $OS_FILE $OS_URL
echo "Unzip openwrt image"
gunzip -c $OS_FILE > $IMG_FILE
echo "$? error code for unzip"
echo "Check for disk to write on"
lsblk
DRIVE_DISK=/dev/sda
rem DRIVE_DISK=/dev/vda
echo "Copying $IMG_FILE to $DRIVE_DISK ..."
dd if=$IMG_FILE bs=1M of=$DRIVE_DISK
echo "Check for disk modification"
lsblk
reboot


rem This installer is marked as failed for now because of the < Wrong EFI loader signature > error
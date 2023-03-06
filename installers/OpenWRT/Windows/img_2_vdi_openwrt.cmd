set IMG_FILE=openwrt-22.03.3-x86-64-generic-ext4-combined.img
set VDI_FILE=openwrt.vdi
VBoxManage convertfromraw --format VDI %IMG_FILE% %VDI_FILE%
VBoxManage modifymedium %VDI_FILE% --resize 128
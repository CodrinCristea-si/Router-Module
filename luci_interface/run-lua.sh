#!/bin/bash
echo "Copying files to remote..."
REMOTE_USER=root
REMOTE_IP=192.168.1.1
REMOTE_PASS=hugbug3578
LUCI_DIR=/usr/lib/lua/luci/
sshpass -p "$REMOTE_PASS" scp -r infectivity_controller $REMOTE_USER@$REMOTE_IP:$LUCI_DIR/controller/
if [ $? -eq 0 ]; then echo "Success"
else echo "Failed"
fi
sshpass -p "$REMOTE_PASS" scp -r infectivity_view $REMOTE_USER@$REMOTE_IP:$LUCI_DIR/view/
if [ $? -eq 0 ]; then echo "Success"
else echo "Failed"
fi
echo "Opening browser..."
BROWSER_PROFILE=luci
firefox -CREATEPROFILE $BROWSER_PROFILE
firefox -P $BROWSER_PROFILE -url $REMOTE_IP

echo "Done"
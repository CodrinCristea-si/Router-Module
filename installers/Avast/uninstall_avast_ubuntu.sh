rem This in an uninstaller for the Avast Antivirus and Services
 
systemctl stop avast
apt purge avast-rest
apt purge avast-fss
apt purge avast
cd /etc/apt/trusted.gpg.d/
rm ./public_key.asc
cd /etc/apt/sources.list.d/
rm avast.list
apt update

rem This uninstaller works as expected
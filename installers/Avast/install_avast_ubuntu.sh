rem Installer for Avast Antivirus and Avast Services
SYSTEM_NAME=$(. /etc/os-release; echo "$ID-$VERSION_CODENAME")
echo "deb https://repo.avcdn.net/linux-av/deb $SYSTEM_NAME release" \
	> /etc/apt/sources.list.d/avast.list
cp ./public_key.asc /etc/apt/trusted.gpg.d/
cp ./licence.avastlic /etc/avast/license.avastlic
apt update
apt install avast
apt install avast-fss
apt install avast-rest
systemctl start avast

rem This installer is marked as failed because of the < No valid licence > error
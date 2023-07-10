echo "Client Side Installer"
echo "Updating apt ..."
apt update
echo "Searching for python version..."
py_version_number=""
py_version_file=.py_version

while read -r line
do
  echo "$line 1"
  if [ -n "$line" ] ; then
    py_version_number="$line"
    break
  fi
done < "$py_version_file"

echo "Python version found: $py_version_number"
python_bin=$(echo python$py_version_number| cut -d . -f 1,2)
echo $python_bin
if [ ! -n  "$python_bin" ];
then
  python_bin=$(echo python$py_version_number| cut -d . -f 1)
fi

py_version=python$py_version_number
echo "Searching for $python_bin..."
is_python_installed=$(which $python_bin)
if [ ! -n "$is_python_installed" ] ;
then
  echo "$py_version not found"
  echo "Preparing environment for building python..."
  python_build_dir=python_build
  mkdir $python_build_dir && cd $python_build_dir
  #echo "Downloading $py_version..."
  py_download_file=Python-$py_version_number.tar.xz
  echo $py_download_file
  wget https://www.python.org/ftp/python/$py_version_number/Python-$py_version_number.tar.xz
  if [ ! -f "$py_download_file" ] ;
  then
    echo "Cannot download Python-$py_version_number"
    exit 1
  fi
  cp $py_download_file $python_build_dir
  echo "Downloading tools for building $py_version..."
  apt install build-essential libssl-dev make
  apt install libbz2-dev libsqlite3-dev libreadline-dev libncurses5-dev libgdbm-dev zlib1g-dev
  py_source_dir=Python-$py_version_number
  echo "Extracting Python package..."
  tar xvf $py_download_file
  cd $py_source_dir
  ./configure --enable-optimizations
  if [ ! $? -eq 0 ]; then
    echo "Python configuration went bad"
    exit 2
  fi
  echo "Building $py_version ... "
  make -j4
  if [ ! $? -eq 0 ]; then
    echo "Python build failed"
    exit 3
  fi
  #echo "Running tests to check Python functionality"
  #make test
  echo "Installing $py_version...."
  make altinstall
  if [ ! $? -eq 0 ]; then
    echo "Python install failed"
    exit 4
  fi
  echo "$py_version installed with success"
  cd ..
else
  echo "$py_version already installed"
fi

echo "Installing pip dependencies..."
pip_file=.pip_requiements
cat $pip_file | while read dependency
do
  $python_bin -m pip install $dependency
done

echo "Installing Clamav..."
apt-get install clamav clamav-daemon -y
echo "Updating signatures..."
sudo touch /run/clamav/clamd.sock
sudo chown clamav:clamav /run/clamav/clamd.sock
sed -ri 's/^(LocalSocket .*)/# \1/g' /etc/clamav/clamd.conf
sudo systemctl restart clamav-daemon.service
sudo systemctl stop clamav-freshclam
sudo rm /var/log/clamav/freshclam.log
sudo freshclam
sudo systemctl start clamav-freshclam
echo "Signatures updated"


#instalation folder size 700mb
echo "Adding task to cron..."
infectivity_folder=$(pwd)
script_path=$(pwd)/start_infectivity.sh
chmod +x $script_path
COMMAND="$script_path $infectivity_folder > /log_cron.txt"
(crontab -l ; echo "* * * * * $COMMAND") | crontab -

echo "Starting the application"
$script_path $infectivity_folder

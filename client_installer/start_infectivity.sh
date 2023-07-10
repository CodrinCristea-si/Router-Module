sudo systemctl stop clamav-freshclam
sudo freshclam
sudo systemctl start clamav-freshclam
infectivity_folder=""
if [ ! -z "$1" ]; then
	infectivity_folder=$1
fi
echo $infectivity_folder
py_version_number=""
py_version_file=$infectivity_folder/.py_version

while read -r line
do
  #echo "$line 1"
  if [ -n "$line" ] ; then
    py_version_number="$line"
    break
  fi
done < "$py_version_file"

#echo "Python version found: $py_version_number"

python_bin=$(echo python$py_version_number| cut -d . -f 1,2)
subnet=192.168.1.0/24
script_path=$infectivity_folder/main.py
echo "$python_bin $script_path -start $subnet"
sudo $python_bin $script_path -start $subnet

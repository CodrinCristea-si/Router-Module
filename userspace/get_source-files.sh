tree -if ./source |grep -E ".*\.c" > a.out
ext_files=$( cat a.out | tr '\n' ' ')
rm a.out
echo $ext_files
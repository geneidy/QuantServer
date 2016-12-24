#!/bin/sh
# Allowing everyone to execute the script, enter:
# 
# chmod +x script.sh
# OR
# 
# chmod 0755 script.sh
# Only allow owner to execute the script, enter:
# 
# chmod 0700 script.sh
# OR
# 
# chmod u=rwx,go= script.sh
# OR
# 
# chmod u+x script.sh
# To view the permissions, use:
# 
# ls -l script.sh
# Set the permissions for the user and the group to read and execute only (no write permission), enter:
# 
# chmod ug=rx script.sh
# Remove read and execute permission for the group and user, enter:
# 
# chmod ug= script.sh

pwd
echo "Running build sequence"
echo "Running distclean"
make distclean
cd GUI/
pwd
echo "Entering GUI/"
echo "Building GUI...Disregard errors: 'undefined reference'"
/usr/bin/qmake-qt4 QtxGui.pro
/usr/bin/qmake-qt4
make
echo "Exiting GUI/"
cd ..
pwd
make all

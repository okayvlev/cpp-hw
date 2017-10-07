#/bin/bash
if cmp -s "$1" "$2" ; then
   echo "OK"
else
   echo "Something changed"
fi
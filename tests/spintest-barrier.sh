#!/bin/bash

set -x

out1=/tmp/spintest-barrier-$$.out

bash -c "time -p tests/spintest-barrier" >& $out1
if [ $? -ne 0 ]; then
    exit 1
fi
usertime=`grep user $out1 | cut -d ' ' -f 2 |cut -d '.' -f 1`
realtime=`grep real $out1 | cut -d ' ' -f 2 |cut -d '.' -f 1`

echo results:
cat $out1

if [ "$realtime" -lt 5 ]; then
    exit 1
fi
if [ "$usertime" -gt 0 ]; then
    exit 1
fi

rm $out1

exit 0

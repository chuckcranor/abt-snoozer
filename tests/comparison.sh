#!/bin/bash

set -x

out1=/tmp/comparison-standard-$$.out
out2=/tmp/comparison-snoozer-$$.out

bash -c "time -p examples/basic-standard" >& $out1
if [ $? -ne 0 ]; then
    exit 1
fi
standardusertime=`grep user $out1 | cut -d ' ' -f 2 |cut -d '.' -f 1`
standardrealtime=`grep real $out1 | cut -d ' ' -f 2 |cut -d '.' -f 1`

bash -c "time -p examples/basic-snoozer" >& $out2
if [ $? -ne 0 ]; then
    exit 1
fi
snoozerusertime=`grep user $out2 | cut -d ' ' -f 2 |cut -d '.' -f 1`
snoozerrealtime=`grep real $out2 | cut -d ' ' -f 2 |cut -d '.' -f 1`

echo standard:
cat $out1

echo snoozer:
cat $out2

# both should take at least 5 real seconds to complete
if [ "$standardrealtime" -lt 5 ]; then
    exit 1
fi
if [ "$snoozerrealtime" -lt 5 ]; then
    exit 1
fi

# standard one should also take at least 4 user seconds
# note: we set 4 as the threshold rather than 5, because it isn't uncommon
# for the test to report 4.9 seconds consumed, and we have truncated the
# decimal in this script.
if [ "$standardusertime" -lt 4 ]; then
    exit 1
fi
# snoozer one should take less than one user second
if [ "$snoozerusertime" -gt 0 ]; then
    exit 1
fi

rm $out1
rm $out2

exit 0

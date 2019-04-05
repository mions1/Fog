#!/bin/bash
INFILE=$1

N=`scavetool list $INFILE | grep serviceTime | cut -d _ -f2`

TMPFILE=/tmp/$$.tmp

for i in $N
do
	rm -f $TMPFILE
	CSVFILE="serviceTime_${i}.csv"
	scavetool vector -p "name(responseTime_$i)" -F csv -O $TMPFILE $INFILE
	fromdos	$TMPFILE
	nl=`cat $TMPFILE | wc -l`
	((nl=nl-1))
	echo "#timeStamp responseTime" > $CSVFILE
	for l in `tail -n-$nl $TMPFILE`
	do
		#echo "line is \"$l\""
		n1=`echo $l| cut -f1 -d,`
		n2=`echo $l| cut -f2 -d,`
		#echo $n1
		#echo $n2
		echo `echo "scale=5; " $n1 "-" $n2 ";" | bc` $n2 >> $CSVFILE
	done
done

rm -f $TMPFILE

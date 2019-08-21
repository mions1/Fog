#!/bin/sh
#for i in results/Fog-q*.sca
for i in results/Fog-mul*-rho*_MG*.sca
do
	BASE=$(echo $i | sed 's/.sca//')
	#Q=$(echo $BASE | cut -d'-' -f2 | cut -d'_' -f2)
	DATA=$$.data
	./meanScalar.py $BASE >> $DATA
	RHO=$(grep rho $DATA | cut -d= -f3 | cut -d' ' -f1)
	T=$(grep avg_responseTime_1 $DATA | cut -d= -f3 | cut -d' ' -f1)
	#DROP=$(grep droppedJobsQueue $DATA | cut -d= -f3 | cut -d' ' -f1)
	DROP=$(grep droppedJobsTotal $DATA | cut -d= -f3 | cut -d' ' -f1)
	JOBS=$(grep totalJobs $DATA | cut -d= -f3 | cut -d' ' -f1)
	#echo "$Q\t$RHO\t$T\t$DROP\t$JOBS"
	echo "$BASE\t$RHO\t$T\t$DROP\t$JOBS"
	rm $DATA
done

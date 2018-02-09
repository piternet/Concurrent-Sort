#!/bin/bash
for i in {1..1}
do
	./gen_test.py $i
	../build/sort < $i".in" > tmp.out
	diff tmp.out $i".out"
	echo $i "OK"
	#rm tmp.out
done

#rm *.in *.out
echo "ls="
ls /dev/shm
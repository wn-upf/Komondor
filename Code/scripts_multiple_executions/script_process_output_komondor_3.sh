#!/bin/bash

# get input files path in folder 'script_input_files'
cd outputs_komondor/sce3/approaches/legacy
pwd

#delete old files
mkdir -p processed_output
rm processed_output/*

echo 'DETECTED KOMONDOR INPUT FILES: '
file_ix=0
while read line
do
	array[ $file_ix ]="$line"
	echo "- ${array[file_ix]}"
	(( file_ix++ ))
done < <(ls)

(( file_ix --));

# processing part 1: extract lines where word "throughput" appears
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
    grep "throughput" ${array[executing_ix]} > processed_output/test_output_$(printf %02d $executing_ix).txt
done

# processing part 2: parse the pre-processed files and generate a csv file with the timestamp and the throughput value
cd processed_output/
file_ix=0
while read line
do
	array[ $file_ix ]="$line"
	(( file_ix++ ))
done < <(ls)
(( file_ix --));
# process the files (remove "=")
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
   while IFS="=" read a b;do
	   echo $a ";" $b >> 1_test_output_$(printf %02d $executing_ix).txt
   done < ${array[executing_ix]}
done

# process the files (select only the timestamp and the throughput value)
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
   while IFS=";" read a b c d e; do
	echo $a ";" $e | sed -e 's/\<Mbps\>//g' >> processed_output_$(printf %02d $executing_ix).txt
   done < 1_${array[executing_ix]}
done

# remove the created auxiliary files
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
    rm test_output_$(printf %02d $executing_ix).txt
    rm 1_test_output_$(printf %02d $executing_ix).txt
done

echo ""
echo 'SCRIPT FINISHED: OUTPUT FILES SAVED IN /processed_output/*'
echo ""
echo ""


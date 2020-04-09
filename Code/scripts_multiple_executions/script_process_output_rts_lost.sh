#!/bin/bash

# get input files path in folder 'script_input_files'
cd ../output/logs_agents/200mbps
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

# processing part 1: extract lines where word "Prob. RTS/CTS lost" appears
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
    grep "Prob. RTS/CTS lost" ${array[executing_ix]} > processed_output/test_output_$(printf %03d $executing_ix).csv
done

# processing part 2: parse the pre-processed files and generate a csv file with the timestamp and the Prob. RTS/CTS lost value
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
	   echo $a ";" $b >> 1_test_output_$(printf %03d $executing_ix).csv
   done < ${array[executing_ix]}
done

# process the files (select only the timestamp and the Prob. RTS/CTS lost value)
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
   while IFS=";" read a b c d e; do
	echo $a ";" $e | sed -e 's/\<Mbps\>//g' >> processed_output_$(printf %03d $executing_ix).csv
   done < 1_${array[executing_ix]}
done

# remove the created auxiliary files
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
    rm test_output_$(printf %03d $executing_ix).csv
    rm 1_test_output_$(printf %03d $executing_ix).csv
done

echo ""
echo 'SCRIPT FINISHED: OUTPUT FILES SAVED IN /processed_output/*'
echo ""
echo ""


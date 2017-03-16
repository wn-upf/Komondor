# compile KOMONDOR
./build_local
echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION... '
# remove old script output file and node logs
rm output/*

# get input files path in folder 'script_input_files'
cd input/script_input_files

echo 'DETECTED KOMONDOR INPUT FILES: '
file_ix=0
while read line
do
	array[ $file_ix ]="$line"
	echo "- ${array[file_ix]}"
	(( file_ix++ ))
done < <(ls)

(( file_ix --));

# execute files

cd ..
cd ..
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./VaquitaSimulation ./input/input_system_conf.csv ./input/script_input_files/${array[executing_ix]} ./output/script_output.txt sim${executing_ix} 1 0 0 0 1000 500
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo 'SCRIPT FINISHED: OUTUP FILE SAVED IN /output/script_output.txt'

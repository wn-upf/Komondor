#!/bin/bash
# Running script through bash, not bash. The solution is as simple as adding the necessary #!/bin/bash

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

# simulation time [s]
sim_time=1000

cd ..
cd ..
touch output/console_logs.txt
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	seed=1992
	echo "  * Seed ${seed}"
./komondor_rts_cts ./input/input_system_conf.csv ./input/script_input_files/${array[executing_ix]} ./output/script_output.txt sim_seed${seed}_${array[executing_ix]} 0 0 0 1 ${sim_time} ${seed} >> output/console_logs.txt
#	seed=42
#	echo "  * Seed ${seed}"
#./komondor_rts_cts ./input/input_system_conf.csv ./input/script_input_files/${array[executing_ix]} ./output/script_output.txt sim_seed${seed}_${array[executing_ix]} 0 0 0 1 ${sim_time} ${seed} >> output/console_logs.txt
#	seed=8080
#	echo "  * Seed ${seed}"
#./komondor_rts_cts ./input/input_system_conf.csv ./input/script_input_files/${array[executing_ix]} ./output/script_output.txt sim_seed${seed}_${array[executing_ix]} 0 0 0 1 ${sim_time} ${seed} >> output/console_logs.txt
# 	seed=608108
#	echo "  * Seed ${seed}"
#./komondor_rts_cts ./input/input_system_conf.csv ./input/script_input_files/${array[executing_ix]} ./output/script_output.txt sim_seed${seed}_${array[executing_ix]} 0 0 0 1 ${sim_time} ${seed} >> output/console_logs.txt

done
echo ""
echo 'SCRIPT FINISHED: OUTUP FILE SAVED IN /output/script_output.txt'
echo ""
echo ""

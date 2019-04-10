# define execution parameters
SIM_TIME=20
SEED=920
# compile KOMONDOR
cd ..
cd ..
cd main
./build_local
echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION... '
cd ..
pwd
# remove old script output file and node logs
rm ./output/*

# get input files path in folder 'script_input_files'
cd input/paper_11ax_sr_tutorial/input_files/toy_scenario_2c
pwd

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
cd ..
pwd
cd ..
pwd
cd main
pwd
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/paper_11ax_sr_tutorial/input_files/input_system_conf_full_buffer.csv ../input/paper_11ax_sr_tutorial/input_files/toy_scenario_2c/${array[executing_ix]} ../output/script_output.txt sim_${array[executing_ix]} 0 0 0 1 $SIM_TIME $SEED >> ../output/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'SCRIPT FINISHED: OUTPUT FILE SAVED IN /output/script_output.txt'
echo ""
echo ""

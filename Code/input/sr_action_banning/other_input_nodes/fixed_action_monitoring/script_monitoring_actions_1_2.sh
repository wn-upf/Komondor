# define execution parameters
SIM_TIME=10
SEED=1992
# compile KOMONDOR
pwd
cd ..
cd ..
cd ..
pwd
cd main
pwd
./build_local
echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION... '
cd ..
pwd
# remove old script output file and node logs
rm output/*

# get input files path in folder 'script_input_files'
cd input/sr_action_banning/fixed_action_monitoring/toy_scenario_1_all_comb_2/
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
cd ..
pwd
cd main
pwd
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/sr_action_banning/fixed_action_monitoring/toy_scenario_1_all_comb_2/${array[executing_ix]} ../input/sr_action_banning/fixed_action_monitoring/agents_files/agents_decentralized_monitor_2.csv ../output/script_output.txt sim_$(printf %02d $executing_ix) 0 0 1 0 1 1 $SIM_TIME $SEED >> ../output/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'SCRIPT FINISHED: OUTUP FILE SAVED IN /output/script_output.txt'
echo ""
echo ""

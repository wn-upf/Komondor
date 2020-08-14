# define execution parameters
SIM_TIME=100
SEED=1992
# compile KOMONDOR
pwd
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

########################################################
# DENSE DEPLOYMENT - LOW TRAFFIC LOAD
########################################################
# get input files path in folder 'script_input_files'
cd input/sr_action_banning/random/multiple_sta/sce2a
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
# execute files (legacy)
cd ..
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
	./komondor_main ../input/sr_action_banning/random/multiple_sta/sce2a/${array[executing_ix]} ../input/sr_action_banning/agents_files/agents_monitoring_random.csv ../output/script_output_legacy_random_multiple_sta.txt sim_${array[executing_ix]} 0 0 0 0 1 0 $SIM_TIME $SEED >> ../output/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'OUTPUT FILE SAVED IN /output/script_output_RND_NSTA_static.txt'
echo ""
echo ""
# execute files with agents (decentralized)
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/sr_action_banning/random/multiple_sta/sce2a/${array[executing_ix]} ../input/sr_action_banning/agents_files/agents_decentralized_random.csv ../output/script_output_selfish_random_multiple_sta.txt sim_${array[executing_ix]} 0 0 0 0 1 0 $SIM_TIME $SEED >> ../output/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'OUTPUT FILE SAVED IN /output/script_output_RND_NSTA_selfish.txt'
echo ""
echo ""
# execute files with agents (decentralized with cost)
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/sr_action_banning/random/multiple_sta/sce2a/${array[executing_ix]} ../input/sr_action_banning/agents_files/agents_centralized_random.csv ../output/script_output_cost_random_multiple_sta.txt sim_${array[executing_ix]} 0 0 0 0 1 0 $SIM_TIME $SEED >> ../output/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'OUTPUT FILE SAVED IN /output/script_output_RND_NSTA_cost.txt'
echo ""
echo ""

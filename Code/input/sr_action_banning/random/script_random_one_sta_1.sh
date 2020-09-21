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
#cd input/sr_action_banning/random/multiple_sta/test_scenario
cd input/sr_action_banning/random/one_sta/sce2
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
cd ..
cd ..
cd ..
cd ..
cd ..
pwd
cd main
pwd
# execute files (legacy)
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/sr_action_banning/random/one_sta/sce2/${array[executing_ix]} ../input/sr_action_banning/agents_files/agents_monitoring_random.csv ../output/script_output_RND_1STA_static.txt sim_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'OUTPUT FILE SAVED IN /output/script_output_RND_1STA_static.txt'
echo ""
echo ""

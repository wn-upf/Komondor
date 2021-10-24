# define execution parameters
SIM_TIME=10
SEED=1991
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

####################
# get input files path in folder 'input_files_new'
cd input/input_ai_challenge_2021
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
	./komondor_main ../input/input_ai_challenge_2021/${array[executing_ix]} ../output/output_11ax_sr_simulations.txt sim_${array[executing_ix]} 0 0 1 $SIM_TIME $SEED >> ../output/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done

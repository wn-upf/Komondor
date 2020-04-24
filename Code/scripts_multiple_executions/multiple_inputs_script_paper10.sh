# define execution parameters
SIM_TIME=100.1
SEED=1992
# compile KOMONDOR
pwd
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
cd input/input_paper10/nodes
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
echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION... '

echo ""
# exploration-first
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: exploration-first"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes/${array[executing_ix]} ../input/input_paper10/agents_action_space2_explfirst.csv ../output/script_output.txt sim_action_space2_explfirst_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# epsilon
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: epsilon"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes/${array[executing_ix]} ../input/input_paper10/agents_action_space2_epsilon.csv ../output/script_output.txt sim_action_space2_epsilon_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. normal
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: TS normal"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes/${array[executing_ix]} ../input/input_paper10/agents_action_space2_tsnormal.csv ../output/script_output.txt sim_action_space2_tsnormal_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. beta
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: TS beta"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes/${array[executing_ix]} ../input/input_paper10/agents_action_space2_tsbeta.csv ../output/script_output.txt sim_action_space2_tsbeta_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

echo 'SCRIPT FINISHED: OUTUP FILE SAVED IN /output/script_output.txt'
echo ""
echo ""

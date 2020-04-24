# define execution parameters

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
cd input/input_paper10/nodes_rq6
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

echo ""
echo ""
echo "***********************************************************************"
echo 'PAPER #10 - reserach question 6: the effect of iteration duration T_it'
echo "***********************************************************************"
echo ""
echo ""
echo ""

# Tit = 100 ms
SIM_TIME=3.21
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: exploration-first"
echo " - Tit = 100 ms"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "     + executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq6/${array[executing_ix]} ../input/input_paper10/agents_action_space2_explfirst_Tit0100ms.csv ../output/script_output.txt sim_action_space2_explfirst_0100ms_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# Tit = 250 ms
SIM_TIME=8.1
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: exploration-first"
echo " - Tit = 250 ms"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "     + executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq6/${array[executing_ix]} ../input/input_paper10/agents_action_space2_explfirst_Tit0250ms.csv ../output/script_output.txt sim_action_space2_explfirst_0250ms_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# Tit = 500 ms
SIM_TIME=16.1
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: exploration-first"
echo " - Tit = 500 ms"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "     + executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq6/${array[executing_ix]} ../input/input_paper10/agents_action_space2_explfirst_Tit0500ms.csv ../output/script_output.txt sim_action_space2_explfirst_0500ms_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# Tit = 1000 ms
SIM_TIME=32.1
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: exploration-first"
echo " - Tit = 1000 ms"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "     + executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq6/${array[executing_ix]} ../input/input_paper10/agents_action_space2_explfirst_Tit1000ms.csv ../output/script_output.txt sim_action_space2_explfirst_1000ms_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

echo 'SCRIPT FINISHED: OUTUP FILE SAVED IN /output/script_output.txt'
echo ""
echo ""

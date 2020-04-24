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
cd input/input_paper10/nodes_rq7
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
echo ""
echo "***********************************************************************"
echo 'PAPER #10 - reserach question 7: effect of the action space size in the MAB learning speed'
echo "***********************************************************************"
echo ""
echo ""
echo ""


# --- Action space 1 ---
echo " **********************"
echo " *** Action space 1 ***"
echo " **********************"
echo ""
# exploration-first
echo " ALGORITHM: exploration-first"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space1/agents_explfirst.csv ../output/script_output.txt sim_action_space1_explfirst_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# epsilon
echo " ALGORITHM: epsilon"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space1/agents_epsilon.csv ../output/script_output.txt sim_action_space1_epsilon_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. normal
echo " ALGORITHM: TS normal"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space1/agents_tsnormal.csv ../output/script_output.txt sim_action_space1_tsnormal_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. beta
echo " ALGORITHM: TS beta"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space1/agents_tsbeta.csv ../output/script_output.txt sim_action_space1_tsbeta_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# UCB
echo " ALGORITHM: UCB"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space1/agents_ucb.csv ../output/script_output.txt sim_action_space1_ucb_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# Exp3
echo " ALGORITHM: Exp3"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space1/agents_exp3.csv ../output/script_output.txt sim_action_space1_exp3_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# --- Action space 2 ---
echo " **********************"
echo " *** Action space 2 ***"
echo " **********************"
echo ""
# exploration-first
echo " ALGORITHM: exploration-first"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space2/agents_explfirst.csv ../output/script_output.txt sim_action_space2_explfirst_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# epsilon
echo " ALGORITHM: epsilon"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space2/agents_epsilon.csv ../output/script_output.txt sim_action_space2_epsilon_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. normal
echo " ALGORITHM: TS normal"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space2/agents_tsnormal.csv ../output/script_output.txt sim_action_space2_tsnormal_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. beta
echo " ALGORITHM: TS beta"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space2/agents_tsbeta.csv ../output/script_output.txt sim_action_space2_tsbeta_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# UCB
echo " ALGORITHM: UCB"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space2/agents_ucb.csv ../output/script_output.txt sim_action_space2_ucb_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# Exp3
echo " ALGORITHM: Exp3"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space2/agents_exp3.csv ../output/script_output.txt sim_action_space2_exp3_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# --- Action space 3 ---
echo " **********************"
echo " *** Action space 3 ***"
echo " **********************"
echo ""
# exploration-first
echo " ALGORITHM: exploration-first"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space3/agents_explfirst.csv ../output/script_output.txt sim_action_space3_explfirst_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# epsilon
echo " ALGORITHM: epsilon"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space3/agents_epsilon.csv ../output/script_output.txt sim_action_space3_epsilon_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. normal
echo " ALGORITHM: TS normal"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space3/agents_tsnormal.csv ../output/script_output.txt sim_action_space3_tsnormal_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. beta
echo " ALGORITHM: TS beta"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space3/agents_tsbeta.csv ../output/script_output.txt sim_action_space3_tsbeta_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# UCB
echo " ALGORITHM: UCB"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space3/agents_ucb.csv ../output/script_output.txt sim_action_space3_ucb_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# Exp3
echo " ALGORITHM: Exp3"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space3/agents_exp3.csv ../output/script_output.txt sim_action_space3_exp3_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# --- Action space 4 ---
echo " **********************"
echo " *** Action space 4 ***"
echo " **********************"
echo ""
# exploration-first
echo " ALGORITHM: exploration-first"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space4/agents_explfirst.csv ../output/script_output.txt sim_action_space4_explfirst_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# epsilon
echo " ALGORITHM: epsilon"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space4/agents_epsilon.csv ../output/script_output.txt sim_action_space4_epsilon_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. normal
echo " ALGORITHM: TS normal"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space4/agents_tsnormal.csv ../output/script_output.txt sim_action_space4_tsnormal_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# T.S. beta
echo " ALGORITHM: TS beta"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space4/agents_tsbeta.csv ../output/script_output.txt sim_action_space4_tsbeta_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# UCB
echo " ALGORITHM: UCB"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space4/agents_ucb.csv ../output/script_output.txt sim_action_space4_ucb_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

# Exp3
echo " ALGORITHM: Exp3"
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo "- executing ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/input_paper10/nodes_rq7/${array[executing_ix]} ../input/input_paper10/agents_rq7/action_space4/agents_exp3.csv ../output/script_output.txt sim_action_space4_exp3_${array[executing_ix]} 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
done
echo ""

echo 'SCRIPT FINISHED: OUTUP FILE SAVED IN /output/script_output.txt'
echo ""
echo ""

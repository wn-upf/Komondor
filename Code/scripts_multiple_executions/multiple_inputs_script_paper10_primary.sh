# define execution parameters
SIM_TIME=100.1
NUM_SEEDS=100
# compile KOMONDOR
pwd
cd ..
pwd
cd main
pwd
./build_local

# remove old script output file and node logs
cd ..
pwd
rm output/*


# execute file
pwd
cd main
pwd

echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION... '

# exploration-first
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: exploration-first"
for (( executing_ix=1; executing_ix < ($NUM_SEEDS + 1); executing_ix++))
do 
	SEED=$executing_ix
	echo "  - executing seed $SEED / $NUM_SEEDS"
	./komondor_main ../input/input_paper10/input_mab_example/input_nodes.csv ../input/input_paper10/input_mab_example/agents_explfirst.csv ../output/script_output.txt sim_explfirst_seed$(printf %03d $SEED) 0 1 0 0 1 $SIM_TIME $SEED >> ../output/logs_console_primary.tx
done
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

# epsilon-greedy
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: epsilon-greedy"
for (( executing_ix=1; executing_ix < ($NUM_SEEDS + 1); executing_ix++))
do 
	SEED=$executing_ix
	echo "  - executing seed $SEED / $NUM_SEEDS"
	./komondor_main ../input/input_paper10/input_mab_example/input_nodes.csv ../input/input_paper10/input_mab_example/agents_epsilon.csv ../output/script_output.txt sim_epsilon_seed$(printf %03d $SEED) 0 1 0 0 1 $SIM_TIME $SEED >> ../output/logs_console_primary.tx
done
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

# T.S. Normal
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: T.S. normal"
for (( executing_ix=1; executing_ix < ($NUM_SEEDS + 1); executing_ix++))
do 
	SEED=$executing_ix
	echo "  - executing seed $SEED / $NUM_SEEDS"
	./komondor_main ../input/input_paper10/input_mab_example/input_nodes.csv ../input/input_paper10/input_mab_example/agents_ts_normal.csv ../output/script_output.txt sim_tsnormal_seed$(printf %03d $SEED) 0 1 0 0 1 $SIM_TIME $SEED >> ../output/logs_console_primary.tx
done

# T.S. Beta
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: T.S. beta"
for (( executing_ix=1; executing_ix < ($NUM_SEEDS + 1); executing_ix++))
do 
	SEED=$executing_ix
	echo "  - executing seed $SEED / $NUM_SEEDS"
	./komondor_main ../input/input_paper10/input_mab_example/input_nodes.csv ../input/input_paper10/input_mab_example/agents_ts_beta.csv ../output/script_output.txt sim_tsbeta_seed$(printf %03d $SEED) 0 1 0 0 1 $SIM_TIME $SEED >> ../output/logs_console_primary.tx
done

# UCB
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: UCB"
for (( executing_ix=1; executing_ix < ($NUM_SEEDS + 1); executing_ix++))
do 
	SEED=$executing_ix
	echo "  - executing seed $SEED / $NUM_SEEDS"
	./komondor_main ../input/input_paper10/input_mab_example/input_nodes.csv ../input/input_paper10/input_mab_example/agents_ucb.csv ../output/script_output.txt sim_ucb_seed$(printf %03d $SEED) 0 1 0 0 1 $SIM_TIME $SEED >> ../output/logs_console_primary.tx
done

# EXP3
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo " ALGORITHM: EXP3"
for (( executing_ix=1; executing_ix < ($NUM_SEEDS + 1); executing_ix++))
do 
	SEED=$executing_ix
	echo "  - executing seed $SEED / $NUM_SEEDS"
	./komondor_main ../input/input_paper10/input_mab_example/input_nodes.csv ../input/input_paper10/input_mab_example/agents_exp3.csv ../output/script_output.txt sim_exp3_$(printf %03d $SEED) 0 1 0 0 1 $SIM_TIME $SEED >> ../output/logs_console_primary.tx
done



echo ""
echo 'SCRIPT FINISHED: OUTUP FILE SAVED IN /output/script_output.txt'
echo ""
echo ""

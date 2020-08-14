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
cd main

echo ""
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "- EXECUTING (1/1)"
./komondor_main ../input/sr_action_banning/other_input_nodes/input_nodes_1.csv ../input/sr_action_banning/agents_files/agents_example.csv ../output/script_testing.txt sim_${array[executing_ix]} 0 0 0 1 1 $SIM_TIME $SEED >> ../output/logs_console.txt
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo ""

echo ""
echo 'OUTPUT FILE SAVED IN /output/script_output_decentralized_dense_low.txt'
echo ""
echo ""

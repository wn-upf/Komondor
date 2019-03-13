# define execution parameters
SIM_TIME=100
SEED=1

# compile KOMONDOR
cd ..
cd main
./build_local
echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION... '
cd ..
# remove old script output file and node logs
rm output/*

##### PART 1: BASIC SCENARIOS
echo ""
echo "++++++++++++++++++++++++++++++++++++"
echo "      PART 3: GRAPH COLORING        "
echo "++++++++++++++++++++++++++++++++++++"
# get input files path
cd input/fg_ml5g/centralized_graph_coloring/random_scenarios
# Detect "nodes" input files
echo 'DETECTED KOMONDOR NODE INPUT FILES: '
nodes_file_ix=0
while read line
do
	array_nodes[ $nodes_file_ix ]="$line"
	echo "- ${array_nodes[nodes_file_ix]}"
	(( nodes_file_ix++ ))
done < <(ls)
(( nodes_file_ix --));

# Execute files
cd ..
cd ..
cd ..
cd ..
cd main
for (( executing_ix_nodes=0; executing_ix_nodes < (nodes_file_ix + 1); executing_ix_nodes++))
do 
	for (( executing_ix_system=0; executing_ix_system < (system_file_ix + 1); executing_ix_system++))
	do 
		echo ""
		echo "- EXECUTING ${array_nodes[executing_ix_nodes]} (${executing_ix_nodes}/${nodes_file_ix})"
		./komondor_main ../input/fg_ml5g/centralized_graph_coloring/input_system_conf.csv ../input/fg_ml5g/centralized_graph_coloring/random_scenarios/${array_nodes[executing_ix_nodes]} ../input/fg_ml5g/centralized_graph_coloring/agents_random_scenario.csv ../output/script_output_basic_scenarios.txt sim_${executing_ix_nodes}.csv 0 0 0 0 1 0 $SIM_TIME $SEED >> ../output/logs_console.txt
	done
done

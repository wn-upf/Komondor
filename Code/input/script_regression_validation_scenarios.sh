# define execution parameters
SIM_TIME=10
SEED=1

# define validation parameters
ALLOWED_ERROR=1	# Allowed error in Mbps
values_basic_scenarios=(88.25 22.80 88.25 22.80)	# Indexes - 1: Sce1a (agg), 2: Sce1a (no agg), 3: Sce1b (agg), 4: Sce1b (no agg)	
declare -A values_complex_scenarios
# Sce2a (agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[0,0]=42.11
values_complex_scenarios[0,1]=41.18
values_complex_scenarios[0,2]=41.70
# Sce2a (no agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[1,0]=9.76
values_complex_scenarios[1,1]=9.76
values_complex_scenarios[1,2]=9.76
# Sce2b (agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[2,0]=108.83
values_complex_scenarios[2,1]=1.53
values_complex_scenarios[2,2]=108.84
# Sce2b (no agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[3,0]=20.87
values_complex_scenarios[3,1]=3.53
values_complex_scenarios[3,2]=20.87
# Sce2c (agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[4,0]=110.21
values_complex_scenarios[4,1]=67.32
values_complex_scenarios[4,2]=110.20
# Sce2c (no agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[5,0]=23.52
values_complex_scenarios[5,1]=18.51
values_complex_scenarios[5,2]=23.53
# Sce2d (agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[6,0]=110.21
values_complex_scenarios[6,1]=110.20
values_complex_scenarios[6,2]=110.22
# Sce2d (no agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[7,0]=23.53
values_complex_scenarios[7,1]=23.52
values_complex_scenarios[7,2]=23.53	

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
echo "      PART 1: BASIC SCENARIOS       "
echo "++++++++++++++++++++++++++++++++++++"
# get input files path
cd input/validation/basic_scenarios/nodes
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
# Detect "system" input files
cd ..
cd system
echo 'DETECTED KOMONDOR SYSTEM INPUT FILES: '
system_file_ix=0
while read line
do
	array_system[ $system_file_ix ]="$line"
	echo "- ${array_system[system_file_ix]}"
	(( system_file_ix++ ))
done < <(ls)
(( system_file_ix --));
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
		echo "- EXECUTING ${array_nodes[executing_ix_nodes]} (${executing_ix_nodes}/${nodes_file_ix}) with ${array_system[executing_ix_system]} (${executing_ix_system}/${system_file_ix})"
		./komondor_main ../input/validation/basic_scenarios/system/${array_system[executing_ix_system]} ../input/validation/basic_scenarios/nodes/${array_nodes[executing_ix_nodes]} ../output/script_output_basic_scenarios.txt sim_${executing_ix_nodes}_${executing_ix_system}.csv 0 0 0 1 $SIM_TIME $SEED >> ../output/logs_console.txt
	done
done

##### PART 2: COMPLEX SCENARIOS
echo ""
echo "++++++++++++++++++++++++++++++++++++"
echo "     PART 2: COMPLEX SCENARIOS      "
echo "++++++++++++++++++++++++++++++++++++"
# get input files path
cd ..
cd input/validation/complex_scenarios/nodes
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
# Detect "system" input files
cd ..
cd system
echo 'DETECTED KOMONDOR SYSTEM INPUT FILES: '
system_file_ix=0
while read line
do
	array_system[ $system_file_ix ]="$line"
	echo "- ${array_system[system_file_ix]}"
	(( system_file_ix++ ))
done < <(ls)
(( system_file_ix --));
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
		echo "- EXECUTING ${array_nodes[executing_ix_nodes]} (${executing_ix_nodes}/${nodes_file_ix}) with ${array_system[executing_ix_system]} (${executing_ix_system}/${system_file_ix})"
		./komondor_main ../input/validation/complex_scenarios/system/${array_system[executing_ix_system]} ../input/validation/complex_scenarios/nodes/${array_nodes[executing_ix_nodes]} ../output/script_output_complex_scenarios.txt sim_${executing_ix_nodes}_${executing_ix_system}.csv 0 0 0 1 $SIM_TIME $SEED >> ../output/logs_console.txt
	done
done

#### VALIDATION
echo ""
echo "++++++++++++++++++++++++++++++++++++"
echo "        VALIDATION RESULTS	  "
echo "++++++++++++++++++++++++++++++++++++"
cd ..
# Declare labels for each scenario (to be used when plotting the results)
declare -a name_basic_scenarios=("1a_aggregation" "1a_no_aggreg. " "1b_aggregation" "1b_no_aggreg. ")
declare -a name_complex_scenarios=("2a_aggregation" "2a_no_aggreg. " "2b_aggregation" "2b_no_aggreg. " "2c_aggregation" "2c_no_aggreg. " "2d_aggregation" "2d_no_aggreg. ")
## PART 1: Basic scenarios
echo ""
echo "-----------------"
echo "Basic scenarios:"
echo "-----------------"
# Read the throughput from output file
file_basic_validation="./output/script_output_basic_scenarios.txt"
ix=0
while IFS= read line
do
        # Obtain the throughput of WLAN A in each scenario
	tpt_wlan_a="$(cut -d';' -f2 <<<"$line")"
	results_basic_scenarios[$ix]=$tpt_wlan_a
	ix=$((ix + 1))
done <"$file_basic_validation"
echo ""
echo "----------------------------"
echo "|    TEST CASE    | RESULT |"
for (( ix=0; ix < 4; ix++))
do
	# Obtain the throughput of WLAN A in each scenario
	a=${results_basic_scenarios[ix]}
	# Compute lower and upper bounds, according to the actual results and the allowed error
	lower_bound=$(echo "${values_basic_scenarios[ix]} - $ALLOWED_ERROR" | bc)
	upper_bound=$(echo "${values_basic_scenarios[ix]} + $ALLOWED_ERROR" | bc)
	# Assess whether the lower and upper bounds are accomplished
	_output_1=`echo "$a >= $lower_bound" | bc`
	_output_2=`echo "$a <= $upper_bound" | bc`
	# Display the results of each test case
	echo "----------------------------"
	if [ $_output_1 == "1" ] && [ $_output_2 == "1" ] ; then    
		echo "| ${name_basic_scenarios[$ix]}  |   OK   |"
	else
		echo "| ${name_basic_scenarios[$ix]}  |   NOK  |"
	fi
done
echo "----------------------------"

## PART 2: Complex scenarios
echo ""
echo "-----------------"
echo "Complex scenarios:"
echo "-----------------"
# Read the throughput from output file
file_complex_validation="./output/script_output_complex_scenarios.txt"
echo "----------------------------"
echo "|    TEST CASE    | RESULT |"
ix=0
while IFS= read line
do
        # Obtain the throughput of each WLAN in each scenario
	tpt_wlan_a="$(cut -d';' -f2 <<<"$line")"
	tpt_wlan_b="$(cut -d';' -f3 <<<"$line")"
	tpt_wlan_c="$(cut -d';' -f4 <<<"$line")"
	# Compute lower and upper bounds, according to the actual results and the allowed error
	lower_bound_1=$(echo "${values_complex_scenarios[$ix,0]} - $ALLOWED_ERROR" | bc)
	upper_bound_1=$(echo "${values_complex_scenarios[$ix,0]} + $ALLOWED_ERROR" | bc)
	lower_bound_2=$(echo "${values_complex_scenarios[$ix,1]} - $ALLOWED_ERROR" | bc)
	upper_bound_2=$(echo "${values_complex_scenarios[$ix,1]} + $ALLOWED_ERROR" | bc)
	lower_bound_3=$(echo "${values_complex_scenarios[$ix,2]} - $ALLOWED_ERROR" | bc)
	upper_bound_3=$(echo "${values_complex_scenarios[$ix,2]} + $ALLOWED_ERROR" | bc)
	# Assess whether the lower and upper bounds are accomplished
	_output_1_1=`echo "$tpt_wlan_a >= $lower_bound_1" | bc`
	_output_1_2=`echo "$tpt_wlan_a <= $upper_bound_1" | bc`
	_output_2_1=`echo "$tpt_wlan_b >= $lower_bound_2" | bc`
	_output_2_2=`echo "$tpt_wlan_b <= $upper_bound_2" | bc`
	_output_3_1=`echo "$tpt_wlan_c >= $lower_bound_3" | bc`
	_output_3_2=`echo "$tpt_wlan_c <= $upper_bound_3" | bc`
	# Display the results of each test case
	echo "----------------------------"
	if [ $_output_1_1 == "1" ] && [ $_output_1_2 == "1" ] && [ $_output_2_1 == "1" ] && [ $_output_2_2 == "1" ] && [ $_output_3_1 == "1" ] && [ $_output_3_2 == "1" ]; then    
		echo "| ${name_complex_scenarios[$ix]}  |   OK   |"
	else
		echo "| ${name_complex_scenarios[$ix]}  |   NOK  |"
	fi
	ix=$((ix + 1))
done <"$file_complex_validation"
echo "----------------------------"

echo ""
echo 'SCRIPT FINISHED: OUTPUT FILE SAVED IN /output/script_output_basic_scenarios.txt and /output/script_output_complex_scenarios.txt'
echo ""
echo ""

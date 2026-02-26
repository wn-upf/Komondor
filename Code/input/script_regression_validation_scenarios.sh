# define execution parameters
SIM_TIME=100
SEED=1

# define validation parameters
ALLOWED_ERROR_TPT=1	# Allowed error in Mbps
# Indexes - 1: Sce1a (no agg), 2: Sce1a (agg), 3: Sce1b (no agg), 4: Sce1b (agg)	
values_basic_scenarios=(22.80 88.25 22.80 88.25)	
declare -A values_complex_scenarios
# Sce2a (no agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[0,0]=10.06
values_complex_scenarios[0,1]=10.05
values_complex_scenarios[0,2]=10.04
# Sce2a (agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[1,0]=49.95
values_complex_scenarios[1,1]=49.45
values_complex_scenarios[1,2]=49.64
# Sce2b (no agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[2,0]=21.69
values_complex_scenarios[2,1]=3.50
values_complex_scenarios[2,2]=21.69
# Sce2b (agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[3,0]=131.04
values_complex_scenarios[3,1]=1.19
values_complex_scenarios[3,2]=131.05
# Sce2c (no agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[4,0]=24.31
values_complex_scenarios[4,1]=18.46
values_complex_scenarios[4,2]=24.30
# Sce2c (agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[5,0]=132.08
values_complex_scenarios[5,1]=65.02
values_complex_scenarios[5,2]=132.10
# Sce2d (no agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[6,0]=24.31
values_complex_scenarios[6,1]=24.30
values_complex_scenarios[6,2]=24.31	
# Sce2d (agg) - Indexes: 0: WLAN_A, 1: WLAN_B, 2: WLAN_C
values_complex_scenarios[7,0]=132.09
values_complex_scenarios[7,1]=132.09
values_complex_scenarios[7,2]=132.09
# Sce3 - Indexes: 0: Sce3a (20MHz), Sce3b (40MHz), Sce3c (80MHz), Sce3d (160MHz)
ALLOWED_ERROR_RSSI=0.1
values_channel_bonding_scenarios=(-108.23 -111.24 -114.25 -117.26)	
# Sce4a (spatial reuse) - Indexes: 0: WLAN_A, 1: WLAN_B
declare -A values_sr_scenarios
values_sr_scenarios[0,0]=30.65
values_sr_scenarios[0,1]=30.62
# Sce4b (no spatial reuse) - Indexes: 0: WLAN_A, 1: WLAN_B
values_sr_scenarios[1,0]=21.131
values_sr_scenarios[1,1]=21.101

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
cd input/validation/basic_scenarios/
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
cd main
for (( executing_ix_nodes=0; executing_ix_nodes < (nodes_file_ix + 1); executing_ix_nodes++))
do 
	echo ""
	echo "- EXECUTING ${array_nodes[executing_ix_nodes]} (${executing_ix_nodes}/${nodes_file_ix})"
	./komondor_main --nodes ../input/validation/basic_scenarios/${array_nodes[executing_ix_nodes]} --out ../output/script_output_basic_scenarios.txt --code sim_${executing_ix_nodes}.csv --logs-sys 1 --logs-node 1 --save-node 0 --time $SIM_TIME --seed $SEED >> ../output/logs_console.txt
done

##### PART 2: COMPLEX SCENARIOS
echo ""
echo "++++++++++++++++++++++++++++++++++++"
echo "     PART 2: COMPLEX SCENARIOS      "
echo "++++++++++++++++++++++++++++++++++++"
# get input files path
cd ..
cd input/validation/complex_scenarios/
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
cd main
for (( executing_ix_nodes=0; executing_ix_nodes < (nodes_file_ix + 1); executing_ix_nodes++))
do 
	echo ""
	echo "- EXECUTING ${array_nodes[executing_ix_nodes]} (${executing_ix_nodes}/${nodes_file_ix})"
	./komondor_main --nodes ../input/validation/complex_scenarios/${array_nodes[executing_ix_nodes]} --out ../output/script_output_complex_scenarios.txt --code sim_${executing_ix_nodes}.csv --logs-sys 1 --logs-node 1 --save-node 0 --time $SIM_TIME --seed $SEED >> ../output/logs_console.txt
done

##### PART 3: CHANNEL BONDING SCENARIOS
echo ""
echo "++++++++++++++++++++++++++++++++++++"
echo "     PART 3: CHANNEL BONDING SCENARIOS      "
echo "++++++++++++++++++++++++++++++++++++"
# get input files path
cd ..
cd input/validation/channel_bonding_scenarios/
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
cd main
for (( executing_ix_nodes=0; executing_ix_nodes < (nodes_file_ix + 1); executing_ix_nodes++))
do 
	echo ""
	echo "- EXECUTING ${array_nodes[executing_ix_nodes]} (${executing_ix_nodes}/${nodes_file_ix})"
	./komondor_main --nodes ../input/validation/channel_bonding_scenarios/${array_nodes[executing_ix_nodes]} --out ../output/script_output_channel_bonding_scenarios.txt --code sim_${executing_ix_nodes}.csv --logs-sys 1 --logs-node 1 --save-node 0 --time $SIM_TIME --seed $SEED >> ../output/logs_console.txt
done

##### PART 4: SPATIAL REUSE SCENARIOS
echo ""
echo "++++++++++++++++++++++++++++++++++++"
echo "     PART 4: SPATIAL REUSE SCENARIOS      "
echo "++++++++++++++++++++++++++++++++++++"
# get input files path
cd ..
cd input/validation/spatial_reuse_scenarios/
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
cd main
for (( executing_ix_nodes=0; executing_ix_nodes < (nodes_file_ix + 1); executing_ix_nodes++))
do 
	echo ""
	echo "- EXECUTING ${array_nodes[executing_ix_nodes]} (${executing_ix_nodes}/${nodes_file_ix})"
	./komondor_main --nodes ../input/validation/spatial_reuse_scenarios/${array_nodes[executing_ix_nodes]} --out ../output/script_output_spatial_reuse_scenarios.txt --code sim_${executing_ix_nodes}.csv --logs-sys 1 --logs-node 1 --save-node 0 --time $SIM_TIME --seed $SEED >> ../output/logs_console.txt
done

#### VALIDATION
echo ""
echo "++++++++++++++++++++++++++++++++++++"
echo "        VALIDATION RESULTS	  "
echo "++++++++++++++++++++++++++++++++++++"
cd ..
# Declare labels for each scenario (to be used when plotting the results)
declare -a name_basic_scenarios=("1a_no_aggreg. " "1a_aggregation" "1b_no_aggreg. " "1b_aggregation")
declare -a name_complex_scenarios=("2a_no_aggreg. " "2a_aggregation" "2b_no_aggreg. " "2b_aggregation" "2c_no_aggreg. " "2c_aggregation" "2d_no_aggreg. " "2d_aggregation")
declare -a name_channel_bonding_scenarios=("3a_20MHz      " "3b_40MHz      " "3c_80MHz      " "3d_160MHz     ")
declare -a name_spatial_reuse_scenarios=("4a_sr         " "4b_no_sr      ")

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
	string1="$(cut -d';' -f2 <<<"$line")" 			# Get raw information separated by ";"
    string2="$(cut -d'{' -f2 <<<"$string1")" 	
    tpt_wlan_a="$(cut -d'}' -f1 <<<"$string2")" 	
	#tpt_wlan_a="$("$string1" | cut -d "{" -f2 | cut -d "}" -f1)" 	# Remove brackets
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
	lower_bound=$(echo "${values_basic_scenarios[ix]} - $ALLOWED_ERROR_TPT" | bc)
	upper_bound=$(echo "${values_basic_scenarios[ix]} + $ALLOWED_ERROR_TPT" | bc)
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
	string_tpt="$(cut -d';' -f2 <<<"$line")"
	string_tpt="$(cut -d'{' -f2 <<<"$string_tpt")"
	string_tpt="$(cut -d'}' -f1 <<<"$string_tpt")"
	tpt_wlan_a="$(cut -d',' -f1 <<<"$string_tpt")"
	tpt_wlan_b="$(cut -d',' -f2 <<<"$string_tpt")"
	tpt_wlan_c="$(cut -d',' -f3 <<<"$string_tpt")"
	# Compute lower and upper bounds, according to the actual results and the allowed error
	lower_bound_1=$(echo "${values_complex_scenarios[$ix,0]} - $ALLOWED_ERROR_TPT" | bc)
	upper_bound_1=$(echo "${values_complex_scenarios[$ix,0]} + $ALLOWED_ERROR_TPT" | bc)
	lower_bound_2=$(echo "${values_complex_scenarios[$ix,1]} - $ALLOWED_ERROR_TPT" | bc)
	upper_bound_2=$(echo "${values_complex_scenarios[$ix,1]} + $ALLOWED_ERROR_TPT" | bc)
	lower_bound_3=$(echo "${values_complex_scenarios[$ix,2]} - $ALLOWED_ERROR_TPT" | bc)
	upper_bound_3=$(echo "${values_complex_scenarios[$ix,2]} + $ALLOWED_ERROR_TPT" | bc)
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

## PART 3: channel bonding scenarios
echo ""
echo "-----------------"
echo "Channel Bonding scenarios:"
echo "-----------------"
# Read the throughput from output file
file_complex_validation="./output/script_output_channel_bonding_scenarios.txt"
echo "----------------------------"
echo "|    TEST CASE    | RESULT |"
ix=0
while IFS= read line
do
	# Obtain the throughput of each WLAN in each scenario
	string_rssi="$(cut -d';' -f3 <<<"$line")"
	string_rssi="$(cut -d'{' -f2 <<<"$string_rssi")"
	string_rssi="$(cut -d'}' -f1 <<<"$string_rssi")"
	rssi_wlan_a="$(cut -d',' -f1 <<<"$string_rssi")"
	# Compute lower and upper bounds, according to the actual results and the allowed error
	lower_bound=$(echo "${values_channel_bonding_scenarios[ix]} - $ALLOWED_ERROR_RSSI" | bc)
	upper_bound=$(echo "${values_channel_bonding_scenarios[ix]} + $ALLOWED_ERROR_RSSI" | bc)
	# Assess whether the lower and upper bounds are accomplished
	_output_1=`echo "$rssi_wlan_a >= $lower_bound" | bc`
	_output_2=`echo "$rssi_wlan_a <= $upper_bound" | bc`
	# Display the results of each test case
	echo "----------------------------"
	if [ $_output_1 == "1" ] && [ $_output_2 == "1" ] ; then    
		echo "| ${name_channel_bonding_scenarios[$ix]}  |   OK   |"
	else
		echo "| ${name_channel_bonding_scenarios[$ix]}  |   NOK  |"
	fi
	ix=$((ix + 1))
done <"$file_complex_validation"
echo "----------------------------"

## PART 4: spatial reuse scenarios
echo ""
echo "-----------------"
echo "Spatial Reuse scenarios:"
echo "-----------------"
# Read the throughput from output file
file_sr_validation="./output/script_output_spatial_reuse_scenarios.txt"
echo "----------------------------"
echo "|    TEST CASE    | RESULT |"
ix=0
while IFS= read line
do
	# Obtain the throughput of each WLAN in each scenario
	string_tpt="$(cut -d';' -f2 <<<"$line")"
	string_tpt="$(cut -d'{' -f2 <<<"$string_tpt")"
	string_tpt="$(cut -d'}' -f1 <<<"$string_tpt")"
	tpt_wlan_a="$(cut -d',' -f1 <<<"$string_tpt")"
	tpt_wlan_b="$(cut -d',' -f2 <<<"$string_tpt")" 
	# Compute lower and upper bounds, according to the actual results and the allowed error
	lower_bound_1=$(echo "${values_sr_scenarios[$ix,0]} - $ALLOWED_ERROR_TPT" | bc)
	upper_bound_1=$(echo "${values_sr_scenarios[$ix,0]} + $ALLOWED_ERROR_TPT" | bc)
	lower_bound_2=$(echo "${values_sr_scenarios[$ix,1]} - $ALLOWED_ERROR_TPT" | bc)
	upper_bound_2=$(echo "${values_sr_scenarios[$ix,1]} + $ALLOWED_ERROR_TPT" | bc)
	# Assess whether the lower and upper bounds are accomplished
	_output_1_1=`echo "$tpt_wlan_a >= $lower_bound_1" | bc`
	_output_1_2=`echo "$tpt_wlan_a <= $upper_bound_1" | bc`
	_output_2_1=`echo "$tpt_wlan_b >= $lower_bound_2" | bc`
	_output_2_2=`echo "$tpt_wlan_b <= $upper_bound_2" | bc`
	# Display the results of each test case
	echo "----------------------------"
	if [ $_output_1_1 == "1" ] && [ $_output_1_2 == "1" ] && [ $_output_2_1 == "1" ] && [ $_output_2_2 == "1" ]; then    
		echo "| ${name_spatial_reuse_scenarios[$ix]}  |   OK   |"
	else
		echo "| ${name_spatial_reuse_scenarios[$ix]}  |   NOK  |"
	fi
	ix=$((ix + 1))
done <"$file_sr_validation"
echo "----------------------------"

echo ""
echo 'SCRIPT FINISHED: OUTPUT FILE SAVED IN /output/script_output_basic_scenarios.txt, /output/script_output_complex_scenarios.txt, /output/script_output_channel_bonding_scenarios.txt and /output/script_output_spatial_reuse_scenarios.txt'
echo ""
echo ""

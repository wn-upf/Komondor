# define execution parameters
SIM_TIME=10
SEED=1992
# compile KOMONDOR
cd ..
cd ..
cd main
./build_local
# PART 1 - TRAFFIC LOAD = 1000 pkt/s
echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION (TRAFFIC LOAD = 1000 pkt/s)... '
cd ..
# remove old script output file and node logs
rm -rf output/*

#create output folder
mkdir output/traffic_load_1000
echo 'Old output files were deleted!'

# get input files path in folder 'script_input_files'
cd input/paper_11ax_sr_tutorial/input_files/random_scenarios/3_random_dense/traffic_load_1000

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
cd ..
cd ..
cd ..
cd ..
cd main
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/paper_11ax_sr_tutorial/input_files/input_system_conf_poisson_buffer.csv ../input/paper_11ax_sr_tutorial/input_files/random_scenarios/3_random_dense/traffic_load_1000/${array[executing_ix]} ../output/traffic_load_1000/script_output.txt sim_${array[executing_ix]} 0 0 0 1 $SIM_TIME $SEED >> ../output/traffic_load_1000/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'SCRIPT FINISHED: OUTPUT FILE SAVED IN /output/traffic_load_1000/script_output.txt'
echo ""
echo ""

# PART 2 - TRAFFIC LOAD = 5000 pkt/s
echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION (TRAFFIC LOAD = 1000 pkt/s)... '
cd ..
pwd
#create output folder
mkdir output/traffic_load_5000

# get input files path in folder 'script_input_files'
cd input/paper_11ax_sr_tutorial/input_files/random_scenarios/3_random_dense/traffic_load_5000
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
cd ..
cd ..
cd ..
cd ..
cd main
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/paper_11ax_sr_tutorial/input_files/input_system_conf_poisson_buffer.csv ../input/paper_11ax_sr_tutorial/input_files/random_scenarios/3_random_dense/traffic_load_5000/${array[executing_ix]} ../output/traffic_load_5000/script_output.txt sim_${array[executing_ix]} 0 0 0 1 $SIM_TIME $SEED >> ../output/traffic_load_5000/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'SCRIPT FINISHED: OUTPUT FILE SAVED IN /output/traffic_load_5000/script_output.txt'
echo ""
echo ""

# PART 3 - TRAFFIC LOAD = FULL
echo 'EXECUTING KOMONDOR SIMULATIONS WITH FULL CONFIGURATION (TRAFFIC LOAD = 1000 pkt/s)... '
cd ..
pwd
#create output folder
mkdir output/traffic_load_10000

# get input files path in folder 'script_input_files'
cd input/paper_11ax_sr_tutorial/input_files/random_scenarios/3_random_dense/traffic_load_1000
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
cd ..
cd ..
cd ..
cd ..
cd main
for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
do 
	echo ""
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo "- EXECUTING ${array[executing_ix]} (${executing_ix}/${file_ix})"
	./komondor_main ../input/paper_11ax_sr_tutorial/input_files/input_system_conf_full_buffer.csv ../input/paper_11ax_sr_tutorial/input_files/random_scenarios/3_random_dense/traffic_load_1000/${array[executing_ix]} ../output/traffic_load_10000/script_output.txt sim_${array[executing_ix]} 0 0 0 1 $SIM_TIME $SEED >> ../output/traffic_load_10000/logs_console.txt
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
done
echo ""
echo 'SCRIPT FINISHED: OUTPUT FILE SAVED IN /output/traffic_load_10000/script_output.txt'
echo ""
echo ""

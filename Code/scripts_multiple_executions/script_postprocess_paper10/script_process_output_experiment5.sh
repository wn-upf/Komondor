# Create folders first

# logs_output_sim_Tit250ms_epsilon_s003_A0_A

echo "creating folders..."


for TIT in "250" "500" "1000"
do

	mkdir ../../output/Tit$(printf %s $TIT)ms
	mv ../../output/logs_output_sim_Tit$(printf %s $TIT)ms_* ../../output/Tit$(printf %s $TIT)ms

	for MAB_ALG in "fixed" "explfirst" "epsilon" "tsnormal" "tsbeta" "ucb" "exp3"
	do

		mkdir ../../output/Tit$(printf %s $TIT)ms/$(printf %s $MAB_ALG)
		mv ../../output/Tit$(printf %s $TIT)ms/logs_output_sim_Tit$(printf %s $TIT)ms_$(printf %s $MAB_ALG)* ../../output/Tit$(printf %s $TIT)ms/$(printf %s $MAB_ALG)			

	
	done

done

# Process outputs in the created folders

echo "processing outputs..."


for TIT in "250" "500" "1000"
do
	
	echo "--- TIT: $(printf %s $TIT) ms ---"


	for MAB_ALG in "fixed" "explfirst" "epsilon" "tsnormal" "tsbeta" "ucb" "exp3"
	do

		echo "------------------------------"
		echo "MAB #$(printf %s $MAB_ALG)"

		cd ../../output/Tit$(printf %s $TIT)ms/$(printf %s $MAB_ALG)
		pwd				

		#delete old files
		mkdir -p processed_output
		rm processed_output/*

		echo 'DETECTED KOMONDOR INPUT FILES: '
		file_ix=0
		while read line
		do
			array[ $file_ix ]="$line"
			echo "- ${array[file_ix]}"
			(( file_ix++ ))
		done < <(ls)

		(( file_ix --));

		# processing part 1: extract lines where word "satisfaction" appears
		for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
		do 
		    grep "satisfaction" ${array[executing_ix]} > processed_output/test_output_$(printf %03d $executing_ix).csv
		done

		# processing part 2: parse the pre-processed files and generate a csv file with the timestamp and the satisfaction value
		cd processed_output/
		file_ix=0
		while read line
		do
			array[ $file_ix ]="$line"
			(( file_ix++ ))
		done < <(ls)
		(( file_ix --));
		# process the files (remove "=")
		for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
		do 
		   while IFS="=" read a b;do
			   echo $a ";" $b >> 1_test_output_$(printf %03d $executing_ix).csv
		   done < ${array[executing_ix]}
		done

		# process the files (select only the timestamp and the satisfaction value)
		for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
		do 
		   while IFS=";" read a b c d e; do
			echo $a ";" $e | sed -e 's/\<Mbps\>//g' >> processed_output_$(printf %03d $executing_ix).csv
		   done < 1_${array[executing_ix]}
		done

		# remove the created auxiliary files
		for (( executing_ix=0; executing_ix < (file_ix + 1); executing_ix++))
		do 
		    rm test_output_$(printf %03d $executing_ix).csv
		    rm 1_test_output_$(printf %03d $executing_ix).csv
		done

		echo '-------------- Folder processed! --------------'

		echo ''
		echo ''
		echo ''
		
		pwd
		cd ../../../../scripts_multiple_executions/script_postprocess_paper10			
					   
	done

done
	

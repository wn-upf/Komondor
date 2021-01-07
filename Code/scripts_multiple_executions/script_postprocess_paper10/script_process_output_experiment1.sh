# Create folders first

echo "creating folders..."

for ACTION_SPACE_ID in "1" "2" "3" "4"
do

	mkdir ../../output/actionspace$(printf %s $ACTION_SPACE_ID)
	mv ../../output/logs_output_sim_actionspace$(printf %s $ACTION_SPACE_ID)_* ../../output/actionspace$(printf %s $ACTION_SPACE_ID)


	for LOAD in 20 50 100 200
	do

		mkdir ../../output/actionspace$(printf %s $ACTION_SPACE_ID)/$(printf %03d $LOAD)mbps
		mv ../../output/actionspace$(printf %s $ACTION_SPACE_ID)/logs_output_sim_actionspace$(printf %s $ACTION_SPACE_ID)_lA$(printf %03d $LOAD)* ../../output/actionspace$(printf %s $ACTION_SPACE_ID)/$(printf %03d $LOAD)mbps

	done

done


# Process outputs in the created folders

echo "processing outputs..."


for ACTION_SPACE_ID in "1" "2" "3" "4"
do

	echo "------------------------------"
	echo "Action space #$(printf %s $ACTION_SPACE_ID)"

	for LOAD in 20 50 100 200
	do

	echo "    * LOAD = $(printf %03d $LOAD) Mbps"

		cd ../../output/actionspace$(printf %s $ACTION_SPACE_ID)/$(printf %03d $LOAD)mbps
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
			   
			done
		done

	done

done  
				



# remove old script output file
rm output/script_output.txt

# get input files path in folder 'script_input_files'
cd input/script_input_files

echo 'Detected input files: '
file_ix=0
while read line
do
	array[ $file_ix ]="$line"
	echo "- ${array[file_ix]}"
	(( file_ix++ ))
done < <(ls)

# execute files

echo 'Executing Vaquita files... '
cd ..
cd ..
for (( executing_ix=0; executing_ix < file_ix; executing_ix++ ))
do 
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
	echo ""
	echo ""
	echo "- Executing ${array[executing_ix]}"
	./VaquitaSimulation 1 455 0 ./input/input_system_conf.csv ./input/script_input_files/${array[executing_ix]} ./output/script_output.txt
done
echo 'Script finished!'

# remove old script output file
rm output/script_output.txt

# get input files path in folder 'script_input_files'
cd input/script_input_files_system

echo 'Detected input files: '
file_ix=0
while read line
do
	array[ $file_ix ]="$line"
	echo "- ${array[file_ix]}"
	(( file_ix++ ))
done < <(ls)

# execute files

echo ''
echo 'Executing Komondor files... '
cd ..
cd ..
for (( executing_ix=0; executing_ix < file_ix; executing_ix++ ))
do 
	echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	echo ""
	echo ""
	echo ""
	echo "- Executing ${array[executing_ix]}"
	
	./Komondor ./input/script_input_files_system/${array[executing_ix]} ./input/input_nodes_conf.csv ./output/script_output.txt 1000 455 1
done
echo 'Script finished!'

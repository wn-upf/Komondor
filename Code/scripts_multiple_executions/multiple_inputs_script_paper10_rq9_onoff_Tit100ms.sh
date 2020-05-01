# define execution parameters
SIM_TIME=100.01
SEED=1992
# compile KOMONDOR
cd ..
cd main
./build_local
echo ""
echo ""
echo "***********************************************************************"
echo "PAPER #10 - reserach question 9: general effect ot Tit"
echo "***********************************************************************"
echo ""
echo ""
echo ""

  
for (( SCENARIO=1; SCENARIO<=100; SCENARIO++ ))
do  
	for TRAFFIC in 1 4
	do

		for (( PRIMARY=0; PRIMARY<=3; PRIMARY++ ))
		do  
	
			for LOAD in 10 20 50 100
			do  

				echo "$SCENARIO - $TRAFFIC - $PRIMARY - $LOAD $"

			done		

		done


	   
	done
done

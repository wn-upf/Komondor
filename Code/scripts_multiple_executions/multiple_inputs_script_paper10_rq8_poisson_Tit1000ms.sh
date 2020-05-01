# define execution parameters
SIM_TIME=1000.01
SEED=1992
# compile KOMONDOR
cd ..
cd main
./build_local
echo ""
echo ""
echo "***********************************************************************"
echo "PAPER #10 - reserach question 8: effect of the traffic model on the reward distribution"
echo "***********************************************************************"
echo ""
echo ""
echo ""

# primary 0
echo "- executing primary 0"
./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_poisson_primary0.csv ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary0.csv ../output/script_output.txt sim_primary0_input_nodes_n10_cb4_lA100mbps_s024_poisson 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
#./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_onoff ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary0.csv ../output/script_output.txt sim_primary0_input_nodes_n10_cb4_lA100mbps_s024_onoff 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
echo ""

# primary 1
echo "- executing primary 1"
./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_poisson_primary1.csv ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary1.csv ../output/script_output.txt sim_primary1_input_nodes_n10_cb4_lA100mbps_s024_poisson 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
#./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_onoff ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary1.csv ../output/script_output.txt sim_primary1_input_nodes_n10_cb4_lA100mbps_s024_onoff 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
echo ""

# primary 2
echo "- executing primary 2"
./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_poisson_primary2.csv ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary2.csv ../output/script_output.txt sim_primary2_input_nodes_n10_cb4_lA100mbps_s024_poisson 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
echo ""

# primary 3
echo "- executing primary 3"
./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_poisson_primary3.csv ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary3.csv ../output/script_output.txt sim_primary3_input_nodes_n10_cb4_lA100mbps_s024_poisson 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
echo ""

# primary 4
echo "- executing primary 4"
./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_poisson_primary4.csv ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary4.csv ../output/script_output.txt sim_primary4_input_nodes_n10_cb4_lA100mbps_s024_poisson 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
echo ""

# primary 5
echo "- executing primary 5"
./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_poisson_primary5.csv ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary5.csv ../output/script_output.txt sim_primary5_input_nodes_n10_cb4_lA100mbps_s024_poisson 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
echo ""

# primary 6
echo "- executing primary 6"
./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_poisson_primary6.csv ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary6.csv ../output/script_output.txt sim_primary6_input_nodes_n10_cb4_lA100mbps_s024_poisson 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
echo ""

# primary 7
echo "- executing primary 7"
./komondor_main ../input/input_paper10/nodes_rq8/input_nodes_n10_cb4_lA100mbps_s024_poisson_primary7.csv ../input/input_paper10/agents_rq8/Tit_1000ms/agents_monitoring_primary7.csv ../output/script_output.txt sim_primary7_input_nodes_n10_cb4_lA100mbps_s024_poisson 0 1 1 1 1 $SIM_TIME $SEED >> ../output/logs_console.tx
echo ""

echo 'SCRIPT FINISHED: OUTUP FILE SAVED IN /output/script_output.txt'
echo ""
echo ""

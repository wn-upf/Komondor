import random
import math
from typing import List
import csv
import os
import locale
import numpy as np
import matplotlib.pyplot as plt

from structures.Wlan import Wlan

# CONSTANTS
RANDOM_SEED = 1
NUM_WLANS = 9
num_cols = 3
num_rows = 3
FREQUENCY_REUSE = 3
NUM_SCENARIOS = 100
PLOT_ENABLED = False
# # Parameters to be changed, according to the desired scenario type
# type_of_scenario = "dense"
# traffic_type = "full"

# Dictionary for WLAN codes
DICTIONARY = [
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
    "M", "N", "O", "P", "Q", "R", "S", "T", "U", "W", "X", "Y", "Z",
    "AA", "BA", "CA", "DA", "EA", "FA", "GA", "HA", "IA", "JA", "KA", "LA",
    "MA", "NA", "OA", "PA", "QA", "RA", "SA", "TA", "UA", "WA", "XA", "YA", "ZA",
    "AB", "BB", "CB", "DB", "EB", "FB", "GB", "HB", "IB", "JB", "KB", "LB",
    "MB", "NB", "OB", "PB", "QB", "RB", "SB", "TB", "UB", "WB", "XB", "YB", "ZB",
    "AC", "BC", "CC", "DC", "EC", "FC", "GC", "HC", "IC", "JC", "KC", "LC",
    "MC", "NC", "OC", "PC", "QC", "RC", "SC", "TC", "UC", "WC", "XC", "YC", "ZC",
    "AD", "BD", "CD", "DD", "ED", "FD", "GD", "HD", "ID", "JD", "KD", "LD",
    "MD", "ND", "OD", "PD", "QD", "RD", "SD", "TD", "UD", "WD", "XD", "YD", "ZD"
]

CSV_SEPARATOR = ";"

def input_attributes(input_path):
    """Reads input attributes from a CSV file.

    Args:
        input_path (str): Path to the input CSV file.

    Raises:
        FileNotFoundError: If the input file is not found.
        csv.Error: If there is an error reading the CSV file.
    """

    print("Reading input file...")

    global map_width, map_height, NUM_WLANS, num_sta_min, num_sta_max, d_min_AP_AP, d_min_AP_STA, \
        d_max_AP_STA, legacy_ratio, c_sys_width, tpc_default_input, cca_default_input, channel_bonding_model, \
        central_freq, traffic_model, traffic_load, packet_length, num_packets_aggregated, capture_effect_model, \
        capture_effect_thr, constant_per, pifs_activated, bss_color_input, srg_input, non_srg_obss_pd_input, srg_obss_pd_input

    try:
        with open(input_path, 'r') as csvfile:
            reader = csv.reader(csvfile, delimiter=';')
            # Skip the first line (assumed to be a comment)
            next(reader)

            # Read the first line of data
            row = next(reader)

            map_width = float(row[0])
            map_height = float(row[1])
            # num_wlans = int(row[2])
            num_sta_min = int(row[3])
            num_sta_max = int(row[4])
            d_min_AP_AP = float(row[5])
            d_min_AP_STA = float(row[6])
            d_max_AP_STA = float(row[7])
            legacy_ratio = float(row[8])
            c_sys_width = int(row[9])
            tpc_default_input = int(row[10])
            cca_default_input = int(row[11])
            channel_bonding_model = int(row[12])
            central_freq = float(row[13])
            traffic_model = int(row[14])
            traffic_load = int(row[15])
            packet_length = int(row[16])
            num_packets_aggregated = int(row[17])
            capture_effect_model = int(row[18])
            capture_effect_thr = float(row[19])
            constant_per = float(row[20])
            pifs_activated = int(row[21])
            bss_color_input = int(row[22])
            srg_input = int(row[23])
            non_srg_obss_pd_input = int(row[24])
            srg_obss_pd_input = int(row[25])

            print(
                "Input:"
                + "\n- map_width: " + str(map_width)
                + "\n- map_height: " + str(map_height)
                + "\n- num_wlans: " + str(NUM_WLANS)
                + "\n- num_sta_min: " + str(num_sta_min)
                + "\n- num_sta_max: " + str(num_sta_max)
                + "\n- d_min_AP_AP: " + str(d_min_AP_AP)
                + "\n- d_min_AP_STA: " + str(d_min_AP_STA)
                + "\n- d_max_AP_STA: " + str(d_max_AP_STA)
                + "\n- legacy_ratio: " + str(legacy_ratio)
                + "\n- c_sys_width: " + str(c_sys_width)
                + "\n- tpc_default_input: " + str(tpc_default_input)
                + "\n- cca_default_input: " + str(cca_default_input)
                + "\n- bss_color_input: " + str(bss_color_input)
                + "\n- srg_input: " + str(srg_input)
                + "\n- non_srg_obss_pd_input: " + str(non_srg_obss_pd_input)
                + "\n- srg_obss_pd_input: " + str(srg_obss_pd_input)
            )

    except:
        print("Error reading the input constructor file")
        exit()


def generate_wlans(backoff_approach, cw_adaptation, cw_min, cw_max, cw_stage,
                   aps_position_list, stas_position_list):
    """Generates WLAN configurations.

    Args:
        backoff_approach (int): Backoff approach.
        cw_adaptation (int): CW adaptation.
        cw_min (int): Minimum CW value.
        cw_max (int): Maximum CW value.
        cw_stage (int): CW stage.
        aps_position_list: List of AP positions.
        stas_position_list (List[float]): List of STA positions.
    """

    wlan_container = []

    for w in range(NUM_WLANS):

        # print(f"Setting WLAN {w}/{num_wlans + 1}")  # Equivalent to System.out.println
        wlan_id = w
        wlan_code = DICTIONARY[w]
        ap_code = "AP_" + wlan_code
        num_stas = len(stas_position_list[w])
        list_sta_code = [None] * num_stas

        for n in range(num_stas):
            list_sta_code[n] = "STA_" + wlan_code + str(n + 1)

        # Channel allocation
        primary_channel = 0 #random.randint(0, c_sys_width)
        min_ch_allowed = primary_channel
        max_ch_allowed = primary_channel

        # ... (rest of the code for setting up wlan_aux)
        wlan_aux = Wlan(wlan_id=wlan_id, wlan_code=wlan_code, num_stas=num_stas, ap_code=ap_code,
                        list_sta_code=list_sta_code, central_frequency=central_freq,
                        channel_bonding_model=channel_bonding_model,
                        primary_channel=primary_channel, min_ch_allowed=min_ch_allowed, max_ch_allowed=max_ch_allowed,
                        x=0, y=0, z=0, capture_effect_model=capture_effect_model, capture_effect_thr=capture_effect_thr,
                        constant_per=constant_per, pifs_activated=pifs_activated)

       # Assign a random channel
        channel_selected = random.randint(0, FREQUENCY_REUSE - 1)
        wlan_aux.primary_channel = channel_selected
        wlan_aux.min_ch_allowed = channel_selected
        wlan_aux.max_ch_allowed = channel_selected

        # Set APs location
        wlan_aux.x = aps_position_list[w][0]
        wlan_aux.y = aps_position_list[w][1]
        wlan_aux.z = aps_position_list[w][2]

        # Set STAs location
        wlan_aux.stas_position_list = stas_position_list[w]

        # Default sensitivity & transmit power
        wlan_aux.cca_default = cca_default_input
        wlan_aux.tpc_default = tpc_default_input

        # Traffic parameters
        wlan_aux.traffic_model = traffic_model
        wlan_aux.traffic_load = traffic_load
        wlan_aux.packet_length = packet_length
        wlan_aux.num_packets_aggregated = num_packets_aggregated

        # Backoff parameters
        wlan_aux.backoff_type = backoff_approach
        wlan_aux.cw_adaptation = cw_adaptation
        wlan_aux.cw_min = cw_min
        wlan_aux.cw_max = cw_max
        wlan_aux.cw_stage = cw_stage

        wlan_container.append(wlan_aux)

    return wlan_container


def generate_file(csv_filepath_output, wlan_container):
    """Generates a simulation file in CSV format.

    Args:
        csv_filepath_output: The path to the output CSV file.
    """

    print("Generating simulation file...")

    # Delete existing file if it exists
    if os.path.exists(csv_filepath_output):
        os.remove(csv_filepath_output)

    # Open the file for writing
    with open(csv_filepath_output, 'w', newline='') as csvfile:

        writer = csv.writer(csvfile, delimiter=CSV_SEPARATOR)

        # Write the header row
        csv_header_line = "node_code" + CSV_SEPARATOR + "node_type" + CSV_SEPARATOR + "wlan_code" + CSV_SEPARATOR + "x(m)" \
                          + CSV_SEPARATOR + "y(m)" + CSV_SEPARATOR + "z(m)" + CSV_SEPARATOR + "central_freq(GHz)" + CSV_SEPARATOR \
                          + "channel_bonding_model" + CSV_SEPARATOR + "primary_channel" + CSV_SEPARATOR + "min_channel_allowed" \
                          + CSV_SEPARATOR + "max_channel_allowed" + CSV_SEPARATOR + "tx_power" + CSV_SEPARATOR + "sensitivity" \
                          + CSV_SEPARATOR + "traffic_model" + CSV_SEPARATOR + "traffic_load(pkt/s)" + CSV_SEPARATOR + "packet_length" \
                          + CSV_SEPARATOR + "num_packets_aggregated" + CSV_SEPARATOR + "capture_effect_model" + CSV_SEPARATOR \
                          + "capture_effect_thr" + CSV_SEPARATOR + "constant_PER" + CSV_SEPARATOR + "pifs_activated" + CSV_SEPARATOR \
                          + "backoff_type" + CSV_SEPARATOR + "cw_adaptation" + CSV_SEPARATOR + "cw_min" + CSV_SEPARATOR + "cw_max" + CSV_SEPARATOR + "cw_stage"

        writer.writerow(csv_header_line.split(CSV_SEPARATOR))

        # Iterate over WLANs
        for wlan in wlan_container:

            # Write AP data
            node_type = 0
            line = get_complete_line(wlan.ap_code, node_type, wlan.wlan_code,
                                     wlan.x, wlan.y, wlan.z, wlan.central_frequency, wlan.channel_bonding_model,
                                     wlan.primary_channel, wlan.min_ch_allowed, wlan.max_ch_allowed,
                                     wlan.tpc_default, wlan.cca_default, wlan.traffic_model,
                                     wlan.traffic_load, wlan.packet_length, wlan.num_packets_aggregated,
                                     wlan.capture_effect_model, wlan.capture_effect_thr, wlan.constant_per,
                                     wlan.pifs_activated, wlan.backoff_type, wlan.cw_adaptation,
                                     wlan.cw_min, wlan.cw_max, wlan.cw_stage)
            writer.writerow(line.split(CSV_SEPARATOR))

            # Write STA data
            node_type = 1
            for n in range(wlan.num_stas):
                line = get_complete_line(wlan.list_sta_code[n], node_type, wlan.wlan_code,
                                         wlan.stas_position_list[n][0], wlan.stas_position_list[n][1],
                                         wlan.stas_position_list[n][2],
                                         wlan.central_frequency, wlan.channel_bonding_model,
                                         wlan.primary_channel, wlan.min_ch_allowed, wlan.max_ch_allowed,
                                         wlan.tpc_default, wlan.cca_default, wlan.traffic_model,
                                         wlan.traffic_load, wlan.packet_length, wlan.num_packets_aggregated,
                                         wlan.capture_effect_model, wlan.capture_effect_thr, wlan.constant_per,
                                         wlan.pifs_activated, wlan.backoff_type, wlan.cw_adaptation,
                                         wlan.cw_min, wlan.cw_max, wlan.cw_stage)
                writer.writerow(line.split(CSV_SEPARATOR))

    print(f"File saved in {csv_filepath_output}")


def get_complete_line(node_code, node_type, wlan_code, x, y, z, central_frequency,
                      channel_bonding_model, primary_channel, min_channel_allowed, max_channel_allowed,
                      tpc_default, cca_default, traffic_model, traffic_load,
                      packet_length, num_packets_aggregated, capture_effect_model,
                      capture_effect_thr, constant_per, pifs_activated,
                      backoff_type, cw_adaptation, cw_min, cw_max, cw_stage):
    """
    Generates a CSV line representing data related to a network node.

    Args:
        node_code (str): Unique code of the node.
        node_type (int): Type of node (0 for AP, 1 for STA).
        wlan_code (str): Code of the WLAN the node is associated with.
        x (float): X coordinate of the node.
        y (float): Y coordinate of the node.
        z (float): Z coordinate of the node.
        central_frequency (float): Central frequency of the WLAN.
        channel_bonding_model (int): Channel bonding model used by the WLAN.
        primary_channel (int): Primary channel of the WLAN.
        min_channel_allowed (int): Minimum allowed channel for the WLAN.
        max_channel_allowed (int): Maximum allowed channel for the WLAN.
        tpc_default (int): Default value for TPC (Transmit Power Control).
        cca_default (int): Default value for CCA (Clear Channel Assessment).
        traffic_model (int): Traffic model used for simulation.
        traffic_load (float): Traffic load on the WLAN.
        packet_length (int): Length of packets transmitted.
        num_packets_aggregated (int): Number of packets aggregated.
        capture_effect_model (int): Capture effect model used.
        capture_effect_thr (float): Capture effect threshold.
        constant_per (float): Constant percentage.
        pifs_activated (int): Whether PIFS (Priority Inter-Frame Spacing) is activated.
        backoff_type (int): Backoff type used.
        cw_adaptation (int): Whether CW (Contention Window) adaptation is enabled.
        cw_min (int): Minimum CW value.
        cw_max (int): Maximum CW value.
        cw_stage (int): Current stage of the CW.

    Returns:
        str: A CSV line representing the node data.
    """

    locale.setlocale(locale.LC_ALL, 'en_UK')  # Set locale for UK number formatting

    line = f"{node_code};{node_type};{wlan_code};{x:.4f};{y:.4f};{z:.4f};{central_frequency};" \
           f"{channel_bonding_model};{primary_channel};{min_channel_allowed};{max_channel_allowed};" \
           f"{tpc_default};{cca_default};{traffic_model};{traffic_load};{packet_length};" \
           f"{num_packets_aggregated};{capture_effect_model};{capture_effect_thr};{constant_per};" \
           f"{pifs_activated};{backoff_type};{cw_adaptation};{cw_min};{cw_max};{cw_stage}"

    return line


def main():
    random.seed(RANDOM_SEED)  # Initialize the random number generator

    # Complete path building
    input_path = f"./input_constructor/input_template_icmlcn.csv"
    print(f"input_path: {input_path}")
    output_path = "./output/*"

    # DEFINE THE BACKOFF VALUES TO BE USED
    backoff_approaches = [0]  # DCF, Tokenized, Deterministic Qualcomm
    cw_adaptation = 1
    cw_min = 0
    cw_max = 15
    cw_stage = 5

    input_attributes(input_path)

    # Create the grid
    aux_count = 0
    grid_centers = [[0.0 for _ in range(2)] for _ in range(NUM_WLANS)]  # Initialize a 2D list

    for c in range(NUM_WLANS):
        grid_centers[c][0] = (c % num_cols) * (map_width / num_cols) + (map_width / num_cols) / 2
        grid_centers[c][1] = (map_height / num_rows) / 2 + aux_count * (map_height / num_rows)
        if (c % num_rows) == (num_rows - 1):
            aux_count += 1

    for n in range(NUM_SCENARIOS):
        # print("- Scenario = ", n)
        # Set APs position
        aps_position_list = []
        stas_position_list = []
        for i in range(NUM_WLANS):
            # print("   + WLAN = ", i)
            x = grid_centers[i][0]
            y = grid_centers[i][1]
            aps_position_list.append(np.array([x, y, 0]))  # Store as a tuple
        # for i in range(NUM_WLANS):
            # Set STAs position
            num_stas = random.randint(num_sta_min, num_sta_max)
            stas_position_list_aux = []
            for j in range(num_stas):
                angle = 360 * random.random()
                rand_value = random.random()
                d_ap_sta = d_min_AP_STA + math.sqrt(rand_value) * (d_max_AP_STA - d_min_AP_STA)
                x = aps_position_list[i][0] + math.cos(math.radians(angle)) * d_ap_sta
                y = aps_position_list[i][1] + math.sin(math.radians(angle)) * d_ap_sta
                stas_position_list_aux.append(np.array([x, y, 0]))
                # Show the distance between the AP and the STA
                distance = np.linalg.norm(aps_position_list[i] - stas_position_list_aux[j])
                # print("      * Distance = ", distance)
            stas_position_list.append(stas_position_list_aux)

        if PLOT_ENABLED:
            colors = ['red', 'green', 'blue', 'cyan', 'magenta', 'yellow', 'black', 'darkorange', 'deeppink']
            plt.figure(figsize=(8, 6))  # Adjust figure size if needed
            for i in range(NUM_WLANS):
                plt.plot(aps_position_list[i][0], aps_position_list[i][1], 'o', color=colors[i], markersize=15)
                for j in range(len(stas_position_list[i])):
                    plt.plot(stas_position_list[i][j][0], stas_position_list[i][j][1], 'x', color=colors[i], markersize=10)
            plt.show()

        # GENERATE EACH .CSV FILE
        for out_ix in range(len(backoff_approaches)):
            wlan_container = generate_wlans(backoff_approaches[out_ix], cw_adaptation, cw_min, cw_max, cw_stage,
                                            aps_position_list,
                                            stas_position_list)
            output_folder = f"./output/input_{NUM_WLANS}bss_fullbuffer/"
            if not os.path.exists(output_folder):
                os.makedirs(output_folder)
            output_path = output_folder+f"input_nodes_sce{n:02d}_FREQUENCY_REUSE_{FREQUENCY_REUSE}_BO_{backoff_approaches[out_ix]}.csv"
            print("output_path:", output_path)
            generate_file(output_path, wlan_container)


# RUN THE CODE
main()

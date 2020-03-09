package old;


import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.text.NumberFormat;
import java.util.Locale;
import java.util.Random;

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
/**
 *
 * @author UPF
 */
public class TwoWLANsGenerator {

    public static double traffic_load_A = 100;

    // USER PARAMETERS (input)
    static double map_width;       // map_width [m]
    static double map_heigth;      // map_heigth [m]
    static int num_wlans;          // number of WLANs (M)
    static int num_sta_min;        // min. number of STAs per WLAN (N_MAX)
    static int num_sta_max;        // max. number of STAs per WLAN (N_MAX)
    static double d_min_AP_AP;     // min. distance between APs
    static double d_min_AP_STA;    // min. distance between AP and STA in same WLAN
    static double d_max_AP_STA;    // max. distance between AP and STA in same WLAN
    static double legacy_ratio;    // proportion of legacy devices (vs. IEEE 802.11 ax)
    static int c_sys_width;         // system channel width
    static int cont_wind;          // contention window (CW)
    static int cont_wind_stage;     // CW stage 
    static int channel_bonding_model;
    static int ieee_protocol;       // IEEE protocol type
    static double traffic_load;
    // -------------------
    static final int destination_id = -1;
    static double tpc_min;
    static double tpc_default;
    static double tpc_max;
    static double cca_min;
    static double cca_default;
    static double cca_max;
    static double tx_antenna_gain;
    static double rx_antenna_gain;
    static int modulation_default;
    static double central_freq;
    static double lambda;

    static final String CSV_SEPARATOR = ";";

    @SuppressWarnings("empty-statement")
    /*
     Read the user input file
     */
    public static void input_attributes(String input_path) throws FileNotFoundException, IOException {

        String line;

        System.out.println("Reading input file...");

        boolean first_line_skipped = false;

        try (BufferedReader br = new BufferedReader(new FileReader(input_path))) {
            while ((line = br.readLine()) != null) {
                if (!first_line_skipped) {
                    // It is a comment --> do nothing
                    first_line_skipped = true;
                } else {

                    String[] node_info = line.split(CSV_SEPARATOR);

                    map_width = Double.parseDouble(node_info[0]);
                    map_heigth = Double.parseDouble(node_info[1]);
                    num_wlans = Integer.parseInt(node_info[2]);
                    num_sta_min = Integer.parseInt(node_info[3]);
                    num_sta_max = Integer.parseInt(node_info[4]);
                    d_min_AP_AP = Double.parseDouble(node_info[5]);
                    d_min_AP_STA = Double.parseDouble(node_info[6]);
                    d_max_AP_STA = Double.parseDouble(node_info[7]);
                    legacy_ratio = Double.parseDouble(node_info[8]);
                    c_sys_width = Integer.parseInt(node_info[9]);
                    cont_wind = Integer.parseInt(node_info[10]);
                    cont_wind_stage = Integer.parseInt(node_info[11]);
                    tpc_min = Double.parseDouble(node_info[12]);
                    tpc_default = Double.parseDouble(node_info[13]);
                    tpc_max = Double.parseDouble(node_info[14]);
                    cca_min = Double.parseDouble(node_info[15]);
                    cca_default = Double.parseDouble(node_info[16]);
                    cca_max = Double.parseDouble(node_info[17]);
                    tx_antenna_gain = Double.parseDouble(node_info[18]);
                    rx_antenna_gain = Double.parseDouble(node_info[19]);
                    channel_bonding_model = Integer.parseInt(node_info[20]);
                    modulation_default = Integer.parseInt(node_info[21]);
                    central_freq = Double.parseDouble(node_info[22]);
                    lambda = Double.parseDouble(node_info[23]);
                    ieee_protocol = Integer.parseInt(node_info[24]);

                    System.out.println(
                            "Input:"
                            + "\n- map_width: " + map_width
                            + "\n- map_heigth: " + map_heigth
                            + "\n- num_wlans: " + num_wlans
                            + "\n- num_sta_min: " + num_sta_min
                            + "\n- num_sta_max: " + num_sta_max
                            + "\n- d_min_AP_AP: " + d_min_AP_AP
                            + "\n- d_min_AP_STA: " + d_min_AP_STA
                            + "\n- d_max_AP_STA: " + d_max_AP_STA
                            + "\n- legacy_ratio: " + legacy_ratio
                            + "\n- c_sys_width: " + c_sys_width
                            + "\n- cont_wind: " + cont_wind);
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void genearate_file(String csv_filepath_output,
            int channel_bonding_model, double traffic_load_B)
            throws UnsupportedEncodingException, FileNotFoundException, IOException {

        System.out.println("Generating simulation file...");

        File fileTemp = new File(csv_filepath_output);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }

        Writer fw = new FileWriter(csv_filepath_output, true);
        BufferedWriter bw = new BufferedWriter(fw);
        PrintWriter out = new PrintWriter(bw);

        String line;
        int line_ix = 0;
        int node_type = 0;
        Wlan wlan;
        double x, y, z = 0;
        double angle = 0;
        double d_ap_sta = 0;
        double rand_value;

        String csv_header_line = "node_code" + CSV_SEPARATOR
                + "node_type" + CSV_SEPARATOR
                + "wlan_code" + CSV_SEPARATOR
                + "destination_id" + CSV_SEPARATOR
                + "x(m)" + CSV_SEPARATOR
                + "y(m)" + CSV_SEPARATOR
                + "z(m)" + CSV_SEPARATOR
                + "primary_channel" + CSV_SEPARATOR
                + "min_channel_allowed" + CSV_SEPARATOR
                + "max_channel_allowed" + CSV_SEPARATOR
                + "cw" + CSV_SEPARATOR
                + "cw_stage" + CSV_SEPARATOR
                + "tpc_min(dBm)" + CSV_SEPARATOR
                + "tpc_default(dBm)" + CSV_SEPARATOR
                + "tpc_max(dBm)" + CSV_SEPARATOR
                + "cca_min(dBm)" + CSV_SEPARATOR
                + "cca_default(dBm)" + CSV_SEPARATOR
                + "cca_max(dBm)" + CSV_SEPARATOR
                + "tx_antenna_gain" + CSV_SEPARATOR
                + "rx_antenna_gain" + CSV_SEPARATOR
                + "channel_bonding_model" + CSV_SEPARATOR
                + "modulation_default" + CSV_SEPARATOR
                + "central_freq (GHz)" + CSV_SEPARATOR
                + "lambda" + CSV_SEPARATOR
                + "ieee_protocol" + CSV_SEPARATOR
                + "traffic_load[pkt/s]";

        // System.out.println(csv_header_line);
        out.println(csv_header_line);

        // AP_A
        Wlan wlan_A = new Wlan();
        wlan_A.ap_code = "AP_A";
        wlan_A.wlan_code = "A";
        wlan_A.x = 0;
        wlan_A.y = 0;
        wlan_A.z = 0;
        wlan_A.primary_channel = 0;
        wlan_A.min_ch_allowed = 0;
        wlan_A.max_ch_allowed = 1;
        wlan_A.channel_bonding_model = channel_bonding_model;
        wlan_A.traffic_load = traffic_load_A;

        line = getCompleteLine(wlan_A.ap_code, 0, wlan_A.wlan_code, wlan_A.x,
                wlan_A.y, wlan_A.z, wlan_A.primary_channel, wlan_A.min_ch_allowed,
                wlan_A.max_ch_allowed, wlan_A.channel_bonding_model, wlan_A.traffic_load);

        out.println(line);

        // STA_A1
        line = getCompleteLine("STA_A1", 1, wlan_A.wlan_code, wlan_A.x,
                wlan_A.y + 1, wlan_A.z, wlan_A.primary_channel,
                wlan_A.min_ch_allowed, wlan_A.max_ch_allowed,
                wlan_A.channel_bonding_model, wlan_A.traffic_load);

        out.println(line);

        // AP_B
        Wlan wlan_B = new Wlan();
        wlan_B.ap_code = "AP_B";
        wlan_B.wlan_code = "B";
        wlan_B.x = 10;
        wlan_B.y = 0;
        wlan_B.z = 0;
        wlan_B.primary_channel = 1;
        wlan_B.min_ch_allowed = 0;
        wlan_B.max_ch_allowed = 1;
        wlan_B.channel_bonding_model = channel_bonding_model;
        wlan_B.traffic_load = traffic_load_B;

        line = getCompleteLine(wlan_B.ap_code, 0, wlan_B.wlan_code, wlan_B.x,
                wlan_B.y, wlan_B.z, wlan_B.primary_channel, wlan_B.min_ch_allowed,
                wlan_B.max_ch_allowed, wlan_B.channel_bonding_model, wlan_B.traffic_load);

        out.println(line);

        // STA_B1
        line = getCompleteLine("STA_B1", 1, wlan_B.wlan_code, wlan_B.x,
                wlan_B.y + 1, wlan_B.z, wlan_B.primary_channel,
                wlan_B.min_ch_allowed, wlan_B.max_ch_allowed,
                wlan_B.channel_bonding_model, wlan_B.traffic_load);

        out.println(line);

        out.close();

        System.out.println("File saved in " + csv_filepath_output);
    }

    static String getCompleteLine(String node_code, int node_type,
            String wlan_code, double x, double y, double z, int primary_channel,
            int min_channel_allowed, int max_channel_allowed, int channel_bonding_model, double traffic_load) {

        // Round position coordinates to limited number of decimals
        NumberFormat nf = NumberFormat.getNumberInstance(Locale.UK);
        nf.setGroupingUsed(true);
        nf.setMaximumFractionDigits(4);

        String line = node_code + CSV_SEPARATOR
                + node_type + CSV_SEPARATOR
                + wlan_code + CSV_SEPARATOR
                + destination_id + CSV_SEPARATOR
                + nf.format(x) + CSV_SEPARATOR
                + nf.format(y) + CSV_SEPARATOR
                + nf.format(z) + CSV_SEPARATOR
                + primary_channel + CSV_SEPARATOR
                + min_channel_allowed + CSV_SEPARATOR
                + max_channel_allowed + CSV_SEPARATOR
                + cont_wind + CSV_SEPARATOR // CW
                + cont_wind_stage + CSV_SEPARATOR // CW's max stage
                + tpc_min + CSV_SEPARATOR
                + tpc_default + CSV_SEPARATOR
                + tpc_max + CSV_SEPARATOR
                + cca_min + CSV_SEPARATOR
                + cca_default + CSV_SEPARATOR
                + cca_max + CSV_SEPARATOR
                + tx_antenna_gain + CSV_SEPARATOR
                + rx_antenna_gain + CSV_SEPARATOR
                + channel_bonding_model + CSV_SEPARATOR
                + modulation_default + CSV_SEPARATOR
                + central_freq + CSV_SEPARATOR
                + lambda + CSV_SEPARATOR
                + ieee_protocol + CSV_SEPARATOR
                + traffic_load;

        return line;
    }

    public static void main(String args[]) throws IOException {

        String input_path = "input_template_rectangular.csv";
        System.out.println("input_path: " + input_path);
        String output_path = "";
        
        input_attributes(input_path);

        double[] traffic_loads_B = {1, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240, 260, 280, 300};

        // OP
        int ch_bonding_model = 0;
        for (int i = 0; i < traffic_loads_B.length; i++) {
            output_path = "input_nodes_2overlapWLANs_cb" + ch_bonding_model
                    + "_loadA_" + String.format("%03d", (int) traffic_load_A)
                    + "_loadB_" + String.format("%03d", (int) traffic_loads_B[i]) + ".csv";
            genearate_file(output_path, ch_bonding_model, traffic_loads_B[i]);
        }

        // SCB
        ch_bonding_model = 2;
        for (int i = 0; i < traffic_loads_B.length; i++) {
            output_path = "input_nodes_2overlapWLANs_cb" + ch_bonding_model
                    + "_loadA_" + String.format("%03d", (int) traffic_load_A)
                    + "_loadB_" + String.format("%03d", (int) traffic_loads_B[i]) + ".csv";
            genearate_file(output_path, ch_bonding_model, traffic_loads_B[i]);
        }

        // AM
        ch_bonding_model = 4;
        for (int i = 0; i < traffic_loads_B.length; i++) {
            output_path = "input_nodes_2overlapWLANs_cb" + ch_bonding_model
                    + "_loadA_" + String.format("%03d", (int) traffic_load_A)
                    + "_loadB_" + String.format("%03d", (int) traffic_loads_B[i]) + ".csv";
            genearate_file(output_path, ch_bonding_model, traffic_loads_B[i]);
        }

        // PU
        ch_bonding_model = 6;
        for (int i = 0; i < traffic_loads_B.length; i++) {
            output_path = "input_nodes_2overlapWLANs_cb" + ch_bonding_model
                    + "_loadA_" + String.format("%03d", (int) traffic_load_A)
                    + "_loadB_" + String.format("%03d", (int) traffic_loads_B[i]) + ".csv";
            genearate_file(output_path, ch_bonding_model, traffic_loads_B[i]);
        }
    }
}

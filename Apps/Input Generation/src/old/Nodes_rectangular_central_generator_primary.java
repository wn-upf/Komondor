package old;


import java.awt.Point;
import java.awt.geom.Point2D;
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
import java.util.concurrent.ThreadLocalRandom;

/**
 *
 * @author Sergio Barrachina-Munoz (sergio.barrachina@upf.edu)
 */
public class Nodes_rectangular_central_generator_primary {

    static Wlan[] wlan_container = null;

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

    static final String DICTIONARY[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
        "M", "N", "O", "P", "Q", "R", "S", "T", "U", "W", "X", "Y", "Z",
        "AA", "BA", "CA", "DA", "EA", "FA", "GA", "HA", "IA", "JA", "KA", "LA",
        "MA", "NA", "OA", "PA", "QA", "RA", "SA", "TA", "UA", "WA", "XA", "YA", "ZA",
        "AB", "BB", "CB", "DB", "EB", "FB", "GB", "HB", "IB", "JB", "KB", "LB",
        "MB", "NB", "OB", "PB", "QB", "RB", "SB", "TB", "UB", "WB", "XB", "YB", "ZB",
        "AC", "BC", "CC", "DC", "EC", "FC", "GC", "HC", "IC", "JC", "KC", "LC",
        "MC", "NC", "OC", "PC", "QC", "RC", "SC", "TC", "UC", "WC", "XC", "YC", "ZC",
        "AD", "BD", "CD", "DD", "ED", "FD", "GD", "HD", "ID", "JD", "KD", "LD",
        "MD", "ND", "OD", "PD", "QD", "RD", "SD", "TD", "UD", "WD", "XD", "YD", "ZD"};

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
                    wlan_container = new Wlan[num_wlans];
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
        }
    }

    public static void generate_wlans() {

        int wlan_counter = 0;

        Wlan wlan_aux;

        int wlan_id;
        String wlan_code;
        int num_stas;
        String ap_code;
        String[] list_sta_code = null;
        int primary_channel;
        int num_channels_allowed_ix;
        int min_ch_allowed = 0;
        int max_ch_allowed = 0;
        boolean wlan_80211ax;   // Legacy or IEEE 802.11 ax
        double x = 0;
        double y = 0;
        double z = 0;
        int channel_bonding_aux;

        double angle = 0;
        double d_ap_sta = 0;
        double rand_value;

        boolean aps_separated;  // Relocate APs until separated

        for (int w = 0; w < num_wlans; w++) {

            System.out.println("Setting WLAN " + w + "/" + num_wlans);

            wlan_id = wlan_counter;
            wlan_code = DICTIONARY[wlan_counter];
            num_stas = ThreadLocalRandom.current().nextInt(num_sta_min, num_sta_max + 1);
            ap_code = "AP_" + wlan_code;
            list_sta_code = new String[num_stas];

            for (int n = 0; n < num_stas; n++) {
                list_sta_code[n] = "STA_" + wlan_code + "" + (n + 1);
            }

            // Channel allocation
            // If WLAN A
            if (w == 0) {
                primary_channel = -1; // To be set later manually
                num_channels_allowed_ix = 3;
            } else {
                primary_channel = ThreadLocalRandom.current().nextInt(0, c_sys_width);
                num_channels_allowed_ix = ThreadLocalRandom.current().nextInt(0, log(c_sys_width, 2) + 1);
            }

            System.out.println("num_channels_allowed: " + num_channels_allowed_ix);

            switch (c_sys_width) {

                case 1: {
                    min_ch_allowed = 0;
                    max_ch_allowed = 0;
                }

                case 2: {

                    if (num_channels_allowed_ix == 0) {
                        min_ch_allowed = primary_channel;
                        max_ch_allowed = primary_channel;
                    } else if (num_channels_allowed_ix == 1) {
                        min_ch_allowed = 0;
                        max_ch_allowed = 1;
                    }
                }

                case 4: {

                    if (num_channels_allowed_ix == 0) {
                        min_ch_allowed = primary_channel;
                        max_ch_allowed = primary_channel;

                    } else if (num_channels_allowed_ix == 1) {
                        if (primary_channel % 2 == 1) {
                            min_ch_allowed = primary_channel - 1;
                            max_ch_allowed = primary_channel;
                        } else {
                            min_ch_allowed = primary_channel;
                            max_ch_allowed = primary_channel + 1;
                        }
                    } else if (num_channels_allowed_ix == 2) {
                        min_ch_allowed = 0;
                    }
                    max_ch_allowed = 3;
                }

                case 8: {

                    if (num_channels_allowed_ix == 0) {
                        min_ch_allowed = primary_channel;
                        max_ch_allowed = primary_channel;
                    } else if (num_channels_allowed_ix == 1) {
                        if (primary_channel % 2 == 1) {
                            min_ch_allowed = primary_channel - 1;
                            max_ch_allowed = primary_channel;
                        } else {
                            min_ch_allowed = primary_channel;
                            max_ch_allowed = primary_channel + 1;
                        }
                    } else if (num_channels_allowed_ix == 2) {
                        if (primary_channel > 3) {	// primary in channel range 4-7
                            min_ch_allowed = 4;
                            max_ch_allowed = 7;
                        } else { // primary in channel range 0-3
                            min_ch_allowed = 0;
                            max_ch_allowed = 3;
                        }
                    } else if (num_channels_allowed_ix == 3) {
                        min_ch_allowed = 0;
                        max_ch_allowed = 7;
                    }
                }
            }

            // IEEE 802.11ax or legacy device
            wlan_80211ax = new Random().nextDouble() <= (1 - legacy_ratio);

            // AP position
            while (true) {

                aps_separated = true;

                // If WLAN A
                if (w == 0) {
                    x = map_width / 2;
                    y = map_heigth / 2;
                    // to be later set
                    channel_bonding_aux = -1;

                } else {
                    x = map_width * new Random().nextDouble();
                    y = map_heigth * new Random().nextDouble();
                    channel_bonding_aux = 2 * ThreadLocalRandom.current().nextInt(0, 4);
                }

                if (w > 0) {
                    for (int w2 = 0; w2 < w; w2++) {

                        double distance = getDistance(wlan_container[w2].x, x,
                                wlan_container[w2].y, y, wlan_container[w2].z, z);

                        if (distance < d_min_AP_AP) {
                            aps_separated = false;
                        }
                    }
                }

                if (aps_separated) {
                    break;
                }
            }
            z = 0;

            wlan_aux = new Wlan(wlan_id, wlan_code, num_stas, ap_code,
                    list_sta_code, primary_channel, min_ch_allowed,
                    max_ch_allowed, wlan_80211ax, x, y, z, channel_bonding_aux, 0);

            Point2D.Double[] stas_position_list = new Point2D.Double[wlan_aux.num_stas];

            // Set STAs location
            for (int n = 0; n < wlan_aux.num_stas; n++) {

                /* Choose the angle uniformly but for the radius an 
                 intermediate value z is generated uniformly between 0 and 1 
                 and then r is calculated as r = sqrt(z)*R.
                 */
                angle = 360 * new Random().nextDouble();
                rand_value = new Random().nextDouble();
                d_ap_sta = d_min_AP_STA
                        + Math.sqrt(rand_value) * (d_max_AP_STA - d_min_AP_STA);

                x = wlan_aux.x + Math.cos(Math.toRadians(angle)) * d_ap_sta;
                y = wlan_aux.y + Math.sin(Math.toRadians(angle)) * d_ap_sta;

                Point2D.Double point = new Point2D.Double();
                point.setLocation(x, y);
                stas_position_list[n] = point;
                wlan_aux.set_stas_positions(stas_position_list);

            }

            wlan_container[w] = wlan_aux;

            wlan_counter++;
        }

        //   System.out.println(line);
    }

    public static void genearate_file(String csv_filepath_output)
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

        for (int w = 0; w < num_wlans; w++) {

            wlan = wlan_container[w];

            node_type = 0;

            line = getCompleteLine(wlan.ap_code, node_type, wlan.wlan_code, wlan.x,
                    wlan.y, wlan.z, wlan.primary_channel, wlan.min_ch_allowed,
                    wlan.max_ch_allowed, wlan.channel_bonding_model, wlan.traffic_load);

            // System.out.println(line);
            out.println(line);

            node_type = 1;

            // Set STAs location
            for (int n = 0; n < wlan.num_stas; n++) {

                line = getCompleteLine(wlan.list_sta_code[n], node_type,
                        wlan.wlan_code, wlan.stas_position_list[n].x, wlan.stas_position_list[n].y, 0, wlan.primary_channel,
                        wlan.min_ch_allowed, wlan.max_ch_allowed,
                        wlan.channel_bonding_model, wlan.traffic_load);

                // System.out.println(line);
                out.println(line);
            }

        }

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

    static int log(int x, int base) {
        return (int) (Math.log(x) / Math.log(base));
    }

    static double getDistance(double x1, double x2, double y1, double y2, double z1, double z2) {
        return Math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
    }

    public static void main(String args[]) throws IOException {

        // CODE FOR JUST ONE OUTPUT
//        String input_path = args[0];        
//        System.out.println("input_path: " + input_path);
//        String output_path = args[1];
//        System.out.println("output_path: " + output_path);
//        input_attributes(input_path);
//        generate_wlans();
//        genearate_file(output_path);
//        System.out.println("Creating map...");
//        String map_viewer_args[] = {output_path};
//        Map_viewer.main(map_viewer_args);
        // CODE FOR SEVERAL OUTPUTS WITH DIFFERENT CHANNEL ALLOCATION AND POSITION
        String input_path = "input_template_rectangular.csv";
        System.out.println("input_path: " + input_path);
        String output_path = "";

        // double[] traffic_loads = {1, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240};
        double[] traffic_loads = {0}; // Test performed for Saturated traffic

        int num_outputs = 200;

        Random r = new Random();

        for (int out_ix = 1; out_ix <= num_outputs; out_ix++) {     // For every deployment

            input_attributes(input_path);

            // Generate static deployment (WLANs position)
            generate_wlans();

            for (int w = 1; w < num_wlans; w++) {
                // In case of random traffic in the rest of WLANs
                double random_traffic_load = traffic_loads[0]
                        + (traffic_loads[traffic_loads.length - 1]
                        - traffic_loads[0]) * r.nextDouble();
                wlan_container[w].traffic_load = random_traffic_load;

            }

            for (int prim_ix = 0; prim_ix <= 7; prim_ix++) {    // For every possible primary channel

                // Set primary of central WLAN
                wlan_container[0].primary_channel = prim_ix;

                // Assign traffic load
                for (int load_ix = 0; load_ix < traffic_loads.length; load_ix++) {  // For every traffic load

//                for (int w = 1; w < num_wlans; w++) {
//
//                    // Determinsitc traffic load
//                    wlan_container[w].traffic_load = traffic_loads[load_ix];
//                }
                    wlan_container[0].traffic_load = traffic_loads[load_ix];

                    // Assign central WLAN's channel bonding model
                    // Always-max
                    channel_bonding_model = 4;
                    wlan_container[0].channel_bonding_model = channel_bonding_model;
                    output_path = "input_nodes_n" + num_wlans + "_s" + out_ix + "_p" + prim_ix + "_cb" + channel_bonding_model
                            + "_load" + String.format("%03d", (int) traffic_loads[load_ix]) + ".csv";
                    System.out.println("output_path: " + output_path);
                    genearate_file(output_path);

                    // SCB
//                    channel_bonding_model = 2;
//                    wlan_container[0].channel_bonding_model = channel_bonding_model;
//                    output_path = "input_nodes_n" + num_wlans + "_s" + out_ix + "_p" + prim_ix + "_cb" + channel_bonding_model
//                            + "_load" + String.format("%03d", (int) traffic_loads[load_ix]) + ".csv";
//                    System.out.println("output_path: " + output_path);
//                    genearate_file(output_path);

                    // OP
//                    channel_bonding_model = 0;
//                    wlan_container[0].channel_bonding_model = channel_bonding_model;
//                    output_path = "input_nodes_n" + num_wlans + "_s" + out_ix + "_p" + prim_ix + "_cb" + channel_bonding_model
//                            + "_load" + String.format("%03d", (int) traffic_loads[load_ix]) + ".csv";
//                    System.out.println("output_path: " + output_path);
//                    genearate_file(output_path);

                    // Prob. uniform
//                    channel_bonding_model = 6;
//                    wlan_container[0].channel_bonding_model = channel_bonding_model;
//                    output_path = "input_nodes_n" + num_wlans + "_s" + out_ix + "_p" + prim_ix + "_cb" + channel_bonding_model
//                            + "_load" + String.format("%03d", (int) traffic_loads[load_ix]) + ".csv";
//                    System.out.println("output_path: " + output_path);
//                    genearate_file(output_path);
                }
            }

        }
    }
}

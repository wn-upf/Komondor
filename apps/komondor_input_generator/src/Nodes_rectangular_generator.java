
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
import static java.lang.Double.max;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

/*
 * Program for generating rectangular node inputs in csv files
 */
/**
 *
 * @author Sergio Barrachina-Munoz (sergio.barrachina@upf.edu)
 */
public class Nodes_rectangular_generator {

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
    static int c_sys_width;     // system channel width
    static int cont_wind;          // contention window (CW)
    static int channel_bonding_model;
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
        "MB", "NB", "OB", "PB", "QB", "RB", "SB", "TB", "UB", "WB", "XB", "YB", "ZB"};

    static final String CSV_SEPARATOR = ";";

    @SuppressWarnings("empty-statement")
    /*
     Read the user input file
     */
    public static void input_attributes(String input_path) throws FileNotFoundException, IOException {

        System.out.println("input_path: " + input_path);

        String line;

        try (BufferedReader br = new BufferedReader(new FileReader(input_path))) {
            while ((line = br.readLine()) != null) {
                if (line.contains("//")) {
                    // It is a comment --> do nothing
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
                    // skip destination id ix 11
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

    public static void generate_wlans() {

        int wlan_counter = 0;

        Wlan wlan_aux;

        int wlan_id;
        String wlan_code;
        int num_stas;
        String ap_code;
        String[] list_sta_code = null;
        int primary_channel;
        int num_channels_allowed;
        int min_ch_allowed = 0;
        int max_ch_allowed = 0;
        boolean wlan_80211ax;   // Legacy or IEEE 802.11 ax
        double x = 0;
        double y = 0;
        double z = 0;

        boolean aps_separated;

        for (int w = 0; w < num_wlans; w++) {

            wlan_id = wlan_counter;
            wlan_code = DICTIONARY[wlan_counter];
            num_stas = ThreadLocalRandom.current().nextInt(num_sta_min, num_sta_max + 1);
            ap_code = "AP_" + wlan_code;
            list_sta_code = new String[num_stas];

            for (int n = 0; n < num_stas; n++) {
                list_sta_code[n] = "STA_" + wlan_code + "" + (n + 1);
            }

            // Channel allocation
            primary_channel = ThreadLocalRandom.current().nextInt(0, c_sys_width);

            num_channels_allowed = ThreadLocalRandom.current().nextInt(0, log(c_sys_width, 2));

            switch (c_sys_width) {

                case 1: {
                    min_ch_allowed = 0;
                    max_ch_allowed = 0;
                }

                case 2: {

                    switch (num_channels_allowed) {

                        case 1: {
                            min_ch_allowed = primary_channel;
                            max_ch_allowed = primary_channel;
                        }

                        case 2: {
                            min_ch_allowed = 0;
                            max_ch_allowed = 1;
                        }
                    }
                }

                case 4: {

                    switch (num_channels_allowed) {

                        case 1: {
                            min_ch_allowed = primary_channel;
                            max_ch_allowed = primary_channel;
                        }

                        case 2: {

                            if (primary_channel % 2 == 1) {
                                min_ch_allowed = primary_channel - 1;
                                max_ch_allowed = primary_channel;
                            } else {
                                min_ch_allowed = primary_channel;
                                max_ch_allowed = primary_channel + 1;
                            }
                        }

                        case 4: {
                            min_ch_allowed = 0;
                            max_ch_allowed = 3;
                        }
                    }
                }

                case 8: {

                    switch (num_channels_allowed) {

                        case 1: {
                            min_ch_allowed = primary_channel;
                            max_ch_allowed = primary_channel;
                        }

                        case 2: {
                            if (primary_channel % 2 == 1) {
                                min_ch_allowed = primary_channel - 1;
                                max_ch_allowed = primary_channel;
                            } else {
                                min_ch_allowed = primary_channel;
                                max_ch_allowed = primary_channel + 1;
                            }
                        }

                        case 4: {
                            if (primary_channel > 3) {	// primary in channel range 4-7
                                min_ch_allowed = 4;
                                max_ch_allowed = 7;
                            } else { // primary in channel range 0-3
                                min_ch_allowed = 0;
                                max_ch_allowed = 3;
                            }
                        }

                        case 8: {
                            min_ch_allowed = 0;
                            max_ch_allowed = 7;
                        }
                    }
                }
            }

            // IEEE 802.11ax or legacy device
            wlan_80211ax = new Random().nextDouble() <= (1 - legacy_ratio);

            // AP position
            while (true) {

                aps_separated = true;

                x = map_width * new Random().nextDouble();
                y = map_heigth * new Random().nextDouble();

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
                    max_ch_allowed, wlan_80211ax, x, y, z, channel_bonding_model);

            wlan_container[w] = wlan_aux;

            wlan_counter++;
        }

        //   System.out.println(line);
    }

    public static void genearate_file(String csv_filepath_output)
            throws UnsupportedEncodingException, FileNotFoundException, IOException {

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
                + "Cwmin" + CSV_SEPARATOR
                + "Cwmax" + CSV_SEPARATOR
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
                + "lambda";
        
        

        System.out.println(csv_header_line);
        out.println(csv_header_line);

        for (int w = 0; w < num_wlans; w++) {

            wlan = wlan_container[w];

            node_type = 0;

            line = getCompleteLine(wlan.ap_code, node_type, wlan.wlan_code, wlan.x,
                    wlan.y, wlan.z, wlan.primary_channel, wlan.min_ch_allowed,
                    wlan.max_ch_allowed, wlan.channel_bonding_model);
            

            System.out.println(line);
            out.println(line);

            node_type = 1;

            for (int n = 0; n < wlan.num_stas; n++) {

                d_ap_sta = d_min_AP_STA + (d_max_AP_STA - d_min_AP_STA) * new Random().nextDouble();
                angle = 360 * new Random().nextDouble();
                x = wlan.x + Math.cos(Math.toRadians(angle)) * d_ap_sta;
                y = wlan.y + Math.sin(Math.toRadians(angle)) * d_ap_sta;
                z = 0;

                line = getCompleteLine(wlan.list_sta_code[n], node_type,
                        wlan.wlan_code, x, y, z, wlan.primary_channel,
                        wlan.min_ch_allowed, wlan.max_ch_allowed,
                        wlan.channel_bonding_model);

                System.out.println(line);
                out.println(line);
            }

        }
        
        out.close();

    }

    static String getCompleteLine(String node_code, int node_type,
            String wlan_code, double x, double y, double z, int primary_channel,
            int min_channel_allowed, int max_channel_allowed, int channel_bonding_model) {

        String line = node_code + CSV_SEPARATOR
                + node_type + CSV_SEPARATOR
                + wlan_code + CSV_SEPARATOR
                + destination_id + CSV_SEPARATOR
                + x + CSV_SEPARATOR
                + y + CSV_SEPARATOR
                + z + CSV_SEPARATOR
                + primary_channel + CSV_SEPARATOR
                + min_channel_allowed + CSV_SEPARATOR
                + max_channel_allowed + CSV_SEPARATOR
                + cont_wind + CSV_SEPARATOR // CW Min
                + cont_wind + CSV_SEPARATOR // CW Max
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
                + lambda;

        return line;
    }

    static int log(int x, int base) {
        return (int) (Math.log(x) / Math.log(base));
    }

    static double getDistance(double x1, double x2, double y1, double y2, double z1, double z2) {
        return Math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
    }

    public static void main(String args[]) throws IOException {

        // String input_path = args[0]
        String input_path = "input_template_rectangular.csv";

        // String input_path = args[1]
        String output_path = "output_rectangular.csv";

        input_attributes(input_path);

        generate_wlans();

        genearate_file(output_path);

    }

}

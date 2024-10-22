from typing import List, Tuple

class Wlan:
    """
    Represents a WLAN (Wireless Local Area Network) with various attributes.
    """

    def __init__(self, wlan_id: int, wlan_code: str, num_stas: int, ap_code: str,
                 list_sta_code: List[str], central_frequency: float, primary_channel: int,
                 min_ch_allowed: int, max_ch_allowed: int, x: float, y: float, z: float,
                 capture_effect_model: int, capture_effect_thr: float, constant_per: float,
                 pifs_activated: int, tpc_default: int = None, channel_bonding_model: int = None,
                 traffic_model: int = None, traffic_load: float = None, packet_length: int = None,
                 num_packets_aggregated: int = None, backoff_type: int = None, cw_adaptation: int = None,
                 cw_min: int = None, cw_max: int = None, cw_stage: int = None, bss_color: int = None,
                 spatial_reuse_group: int = None, non_srg_obss_pd: int = None, srg_obss_pd: int = None):
        """
        Initializes a Wlan object.

        Args:
            wlan_id: The WLAN ID.
            wlan_code: The code of the WLAN.
            num_stas: The number of STAs in the WLAN (AP not included).
            ap_code: The ID of the Access Point.
            list_sta_code: A list of STAs IDs belonging to the WLAN.
            central_frequency: The central frequency of the WLAN.
            primary_channel: The primary channel of the WLAN.
            min_ch_allowed: The minimum allowed channel.
            max_ch_allowed: The maximum allowed channel.
            x: The x-coordinate of the WLAN.
            y: The y-coordinate of the WLAN.
            z: The z-coordinate of the WLAN.
            capture_effect_model: The capture effect model.
            capture_effect_thr: The capture effect threshold.
            constant_per: The constant percentage.
            pifs_activated: Whether PIFS is activated.
            tpc_default: The default TPC value (optional).
            channel_bonding_model: The channel bonding model (optional).
            traffic_model: The traffic model (optional).
            traffic_load: The traffic load (optional).
            packet_length: The packet length (optional).
            num_packets_aggregated: The number of packets aggregated (optional).
            backoff_type: The backoff type (optional).
            cw_adaptation: The CW adaptation (optional).
            cw_min: The minimum CW value (optional).
            cw_max: The maximum CW value (optional).
            cw_stage: The CW stage (optional).
            bss_color: The BSS color (optional).
            spatial_reuse_group: The spatial reuse group (optional).
            non_srg_obss_pd: The non-SRG OBSS PD (optional).
            srg_obss_pd: The SRG OBSS PD (optional).
        """

        self.wlan_id = wlan_id
        self.wlan_code = wlan_code
        self.num_stas = num_stas
        self.ap_code = ap_code
        self.list_sta_code = list_sta_code
        self.x = x
        self.y = y
        self.z = z
        self.central_frequency = central_frequency
        self.channel_bonding_model = channel_bonding_model
        self.primary_channel = primary_channel
        self.min_ch_allowed = min_ch_allowed
        self.max_ch_allowed = max_ch_allowed
        self.traffic_model = traffic_model
        self.traffic_load = traffic_load
        self.packet_length = packet_length
        self.num_packets_aggregated = num_packets_aggregated
        self.capture_effect_model = capture_effect_model
        self.capture_effect_thr = capture_effect_thr
        self.constant_per = constant_per
        self.pifs_activated = pifs_activated
        self.tpc_default = tpc_default
        self.bss_color = bss_color
        self.spatial_reuse_group = spatial_reuse_group
        self.non_srg_obss_pd = non_srg_obss_pd
        self.srg_obss_pd = srg_obss_pd
        self.stas_position_list = None
#!/usr/bin/env python
"""plot_beams.py — Visualise ULA beam patterns for a Komondor static scenario.

Usage
-----
  py plot_beams.py --nodes <nodes.csv> [--mapc <mapc.csv>]
                   [--out <fig.png>] [--out-overlay <fig2.png>]

  --nodes       Node CSV (semicolon-delimited, same format as Komondor inputs).
                BF columns (32-35): bf_enabled;bf_N_elements;bf_d_spacing;bf_az_main_deg
  --mapc        Optional MAPC config CSV.  Format:
                  GroupID;Method;CoordinatedBSSIds;ExtraParams
                Used to derive null directions for the coordinator AP
                (nulls toward all peer APs in the group).
  --out         Save Figure 1 (floor plan + individual polars) to file.
  --out-overlay Save Figure 2 (beams superposed on the map) to file.

Beam model (mirrors beamforming_methods.h exactly)
--------------------------------------------------
  ULA phase:   phi_n = 2*pi*d*n*sin(az)
  Beam weight: projection null-steerer (LCMV/Capon-style)
  Gain:        |AF(az)|^2 = |w' * a(az)|^2  (linear; 0 dB = unity)
"""

import argparse
import math
import sys
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

# ---------------------------------------------------------------------------
# CSV helpers
# ---------------------------------------------------------------------------

def parse_csv(path):
    """Return (header_list, list_of_dicts) from a semicolon-delimited CSV."""
    with open(path, 'r') as f:
        lines = [l.rstrip('\n').rstrip('\r') for l in f if l.strip()]
    if not lines:
        return [], []
    headers = [h.strip() for h in lines[0].split(';')]
    rows = []
    for line in lines[1:]:
        parts = [p.strip() for p in line.split(';')]
        row = {}
        for i, h in enumerate(headers):
            row[h] = parts[i] if i < len(parts) else ''
        rows.append(row)
    return headers, rows


def field(row, key, default=None):
    v = row.get(key, '')
    if v == '' or v is None:
        return default
    return v


def flt(row, key, default=0.0):
    v = field(row, key)
    try:
        return float(v)
    except (TypeError, ValueError):
        return default


def integer(row, key, default=0):
    v = field(row, key)
    try:
        return int(v)
    except (TypeError, ValueError):
        return default

# ---------------------------------------------------------------------------
# Beamforming math (mirrors beamforming_methods.h)
# ---------------------------------------------------------------------------

def ula_steering(az_rad, N, d):
    """Return complex steering vector (length N) for azimuth az_rad."""
    n = np.arange(N, dtype=float)
    phase = 2.0 * math.pi * d * n * math.sin(az_rad)
    return np.cos(phase) + 1j * np.sin(phase)


def compute_beam_weights(az_main_rad, null_az_rads, N, d):
    """Projection null-steerer; returns weight vector (complex, length N)."""
    if N <= 1:
        return np.array([1.0 + 0j])

    w = ula_steering(az_main_rad, N, d).copy()

    for az_null in null_az_rads:
        ak = ula_steering(az_null, N, d)
        # proj_scalar = (ak^H . w) / (ak^H . ak)
        ps = np.dot(ak.conj(), w) / np.dot(ak.conj(), ak)
        w = w - ps * ak

    # Normalize: w^H . a_des = 1
    a_des = ula_steering(az_main_rad, N, d)
    scale = np.dot(w.conj(), a_des)
    if abs(scale) < 1e-15:
        scale = 1e-15
    w = w / scale
    return w


def beam_gain_pattern(az_main_rad, null_az_rads, N, d, n_points=3600):
    """Return (az_rad_array, linear_gain_array) over 0..2*pi."""
    az_arr = np.linspace(0, 2 * math.pi, n_points, endpoint=False)
    return az_arr, beam_gain_at_azimuths(az_main_rad, null_az_rads, N, d, az_arr)


def beam_gain_at_azimuths(az_main_rad, null_az_rads, N, d, az_array):
    """Vectorized beam gain for an array of azimuths (radians).

    Returns linear gain array (same shape as az_array).
    Much faster than calling ula_steering in a Python loop.
    """
    w = compute_beam_weights(az_main_rad, null_az_rads, N, d)
    az_flat = np.asarray(az_array).ravel()
    n_idx = np.arange(N, dtype=float)
    # phase[i, n] = 2*pi*d*n*sin(az_i)
    phase = 2.0 * math.pi * d * np.outer(np.sin(az_flat), n_idx)  # (M, N)
    a = np.cos(phase) + 1j * np.sin(phase)                         # (M, N)
    af = a @ w.conj()                                               # (M,)
    gain = (np.abs(af) ** 2).reshape(np.asarray(az_array).shape)
    return gain

# ---------------------------------------------------------------------------
# Node / scenario parsing
# ---------------------------------------------------------------------------

def parse_nodes(nodes_path):
    """Return list of node dicts with numeric fields filled in."""
    _, rows = parse_csv(nodes_path)
    nodes = []
    for r in rows:
        n = {
            'code':      field(r, 'node_code', '?'),
            'type':      integer(r, 'node_type', 1),  # 0=AP, 1=STA
            'wlan':      field(r, 'wlan_code', '?'),
            'x':         flt(r, 'x(m)'),
            'y':         flt(r, 'y(m)'),
            'z':         flt(r, 'z(m)'),
            'tx_power':  flt(r, 'tx_power', 20.0),          # dBm
            'freq_ghz':  flt(r, 'central_freq (GHz)', 5.0),
            'bf_enabled':  integer(r, 'bf_enabled', 0),
            'bf_N':        integer(r, 'bf_N_elements', 1),
            'bf_d':        flt(r, 'bf_d_spacing', 0.5),
            'bf_az_deg':   flt(r, 'bf_az_main_deg', 0.0),
        }
        nodes.append(n)
    return nodes


def parse_mapc(mapc_path):
    """Return list of group dicts: {group_id, method, wlan_codes:[str]}."""
    _, rows = parse_csv(mapc_path)
    groups = []
    for r in rows:
        codes_raw = field(r, 'CoordinatedBSSIds', '')
        codes = [c.strip() for c in codes_raw.split(',') if c.strip()]
        groups.append({
            'group_id': field(r, 'GroupID', '?'),
            'method':   field(r, 'Method', ''),
            'wlan_codes': codes,
        })
    return groups

# ---------------------------------------------------------------------------
# Build per-AP beam specs
# ---------------------------------------------------------------------------

def build_ap_beam_specs(nodes, mapc_groups):
    """
    Return list of dicts, one per BF-enabled AP:
      {node, az_main_rad, null_az_rads, null_labels, sta_az_rad, sta_dist}
    """
    # Index nodes
    ap_by_wlan  = {n['wlan']: n for n in nodes if n['type'] == 0}
    sta_by_wlan = {n['wlan']: n for n in nodes if n['type'] == 1}

    # Build null sets from MAPC groups
    # Key: AP wlan_code -> list of peer AP wlan codes to null
    null_map = {}  # wlan_code -> [peer_wlan_code, ...]
    for g in mapc_groups:
        codes = g['wlan_codes']
        for c in codes:
            if c not in null_map:
                null_map[c] = []
            for peer in codes:
                if peer != c and peer not in null_map[c]:
                    null_map[c].append(peer)

    specs = []
    for ap in nodes:
        if ap['type'] != 0 or not ap['bf_enabled']:
            continue

        N = max(ap['bf_N'], 1)
        d = ap['bf_d']
        az_main_deg = ap['bf_az_deg']

        # Determine main beam azimuth:
        # If bf_az_main_deg == 0.0 (default) and there is an associated STA,
        # override with the actual direction toward the STA.
        sta = sta_by_wlan.get(ap['wlan'])
        sta_az_rad = None
        sta_dist   = None
        if sta is not None:
            dx = sta['x'] - ap['x']
            dy = sta['y'] - ap['y']
            sta_az_rad = math.atan2(dy, dx)
            sta_dist   = math.hypot(dx, dy)
            if az_main_deg == 0.0:
                az_main_deg = math.degrees(sta_az_rad)

        az_main_rad = math.radians(az_main_deg)

        # Null directions toward peer STAs (the receivers that experience our
        # interference), mirroring the corrected simulator behaviour.
        null_az_rads  = []
        null_labels   = []
        for peer_wlan in null_map.get(ap['wlan'], []):
            peer_sta = sta_by_wlan.get(peer_wlan)
            if peer_sta is None:
                # Fallback: no STA found for this WLAN — skip null
                continue
            dx = peer_sta['x'] - ap['x']
            dy = peer_sta['y'] - ap['y']
            null_az = math.atan2(dy, dx)
            null_az_rads.append(null_az)
            null_labels.append(peer_sta['code'])

        specs.append({
            'node':          ap,
            'az_main_rad':   az_main_rad,
            'null_az_rads':  null_az_rads,
            'null_labels':   null_labels,
            'sta_az_rad':    sta_az_rad,
            'sta_dist':      sta_dist,
        })
    return specs

# ---------------------------------------------------------------------------
# Plotting helpers
# ---------------------------------------------------------------------------

COLORS = ['tab:blue', 'tab:orange', 'tab:green', 'tab:red',
          'tab:purple', 'tab:brown', 'tab:pink', 'tab:cyan']


def plot_floor_plan(ax, nodes, specs):
    """Draw 2-D node layout with beam direction arrows."""
    wlan_codes = sorted(set(n['wlan'] for n in nodes))
    color_map  = {w: COLORS[i % len(COLORS)] for i, w in enumerate(wlan_codes)}

    # Node scatter
    for n in nodes:
        color = color_map[n['wlan']]
        marker = 's' if n['type'] == 0 else 'o'
        ax.scatter(n['x'], n['y'], c=color, marker=marker, s=80, zorder=3)
        ax.annotate(n['code'], (n['x'], n['y']),
                    textcoords='offset points', xytext=(5, 4),
                    fontsize=7, color=color)

    # Beam main direction arrows + null markers
    arrow_len = 1.2
    for sp in specs:
        ap    = sp['node']
        color = color_map[ap['wlan']]
        az    = sp['az_main_rad']
        ax.annotate('', xy=(ap['x'] + arrow_len * math.cos(az),
                             ap['y'] + arrow_len * math.sin(az)),
                    xytext=(ap['x'], ap['y']),
                    arrowprops=dict(arrowstyle='->', color=color, lw=1.5))
        for null_az in sp['null_az_rads']:
            nx = ap['x'] + arrow_len * 0.7 * math.cos(null_az)
            ny = ap['y'] + arrow_len * 0.7 * math.sin(null_az)
            ax.plot([ap['x'], nx], [ap['y'], ny], '--',
                    color='gray', lw=0.8, alpha=0.6)

    # Legend patches
    handles = [mpatches.Patch(color=color_map[w], label='WLAN ' + w)
               for w in wlan_codes]
    handles += [
        plt.Line2D([0], [0], marker='s', color='w', markerfacecolor='k',
                   markersize=8, label='AP'),
        plt.Line2D([0], [0], marker='o', color='w', markerfacecolor='k',
                   markersize=8, label='STA'),
        matplotlib.patches.FancyArrow(0, 0, 1, 0, color='k',
                                      width=0.02, label='Main beam'),
        plt.Line2D([0], [0], ls='--', color='gray', label='Null direction'),
    ]
    ax.legend(handles=handles, fontsize=7, loc='upper right')
    ax.set_aspect('equal')
    ax.grid(True, ls=':', lw=0.5)
    ax.set_xlabel('x (m)')
    ax.set_ylabel('y (m)')
    ax.set_title('Floor plan')


def plot_polar_beam(ax, sp, color, db_floor=-40.0):
    """Draw polar beam pattern on ax (already a polar axes)."""
    ap    = sp['node']
    N     = max(ap['bf_N'], 1)
    d     = ap['bf_d']

    az_arr, gain = beam_gain_pattern(
        sp['az_main_rad'], sp['null_az_rads'], N, d)

    gain_db = 10.0 * np.log10(np.maximum(gain, 10 ** (db_floor / 10.0)))

    # Shift so 0 dB maps to 0 on the polar radius
    r = gain_db - db_floor       # all non-negative

    label = '{} (N={}, d={:.2f}λ)'.format(ap['code'], N, d)
    ax.plot(az_arr, r, color=color, lw=1.5, label=label)

    # Mark main beam
    ax.plot([sp['az_main_rad']], [r[int(sp['az_main_rad'] /
            (2 * math.pi) * len(az_arr)) % len(az_arr)]],
            'o', color=color, ms=5)

    # Mark nulls
    for null_az in sp['null_az_rads']:
        idx = int(null_az / (2 * math.pi) * len(az_arr)) % len(az_arr)
        ax.plot([null_az], [r[idx]], 'x', color='gray', ms=8, mew=1.5)

    # Tick labels in degrees
    ax.set_theta_zero_location('E')
    ax.set_theta_direction(1)

    # Radial ticks: label as dB from db_floor to 0 dB
    n_ticks  = 5
    r_ticks  = np.linspace(0, -db_floor, n_ticks)
    r_labels = ['{:.0f} dB'.format(t + db_floor) for t in r_ticks]
    ax.set_yticks(r_ticks)
    ax.set_yticklabels(r_labels, fontsize=6)
    ax.set_ylim(0, -db_floor + 3)


# ---------------------------------------------------------------------------
# SINR heatmap computation
# ---------------------------------------------------------------------------

def compute_sinr_grid(specs, xx, yy, noise_dbm, alpha):
    """Compute SINR [dB] for the best-serving BF-enabled AP at each grid point.

    Path loss model (log-distance):
        PL_dB(d) = PL_1m + 10 * alpha * log10(d)
        PL_1m    = 20 * log10(4*pi*f/c)   [free-space ref at 1 m]
    Beam gain is applied for DATA-direction beams (as in the simulator).
    Noise and interference are in mW (consistent with dBm tx_power).
    """
    noise_mw = 10.0 ** (noise_dbm / 10.0)
    n_pts = xx.size
    gx = xx.ravel()
    gy = yy.ravel()

    rx_mw = np.zeros((len(specs), n_pts))

    for i, sp in enumerate(specs):
        ap       = sp['node']
        tx_dbm   = ap['tx_power']
        freq_ghz = ap['freq_ghz']
        c_speed  = 3e8
        f_hz     = freq_ghz * 1e9
        pl_1m_db = 20.0 * math.log10(4.0 * math.pi * f_hz / c_speed)

        dx   = gx - ap['x']
        dy   = gy - ap['y']
        dist = np.maximum(np.sqrt(dx * dx + dy * dy), 0.05)  # clamp 5 cm
        pl_db = pl_1m_db + 10.0 * alpha * np.log10(dist)

        N         = max(ap['bf_N'], 1)
        d_spacing = ap['bf_d']
        az_grid   = np.arctan2(dy, dx)
        gain_lin  = beam_gain_at_azimuths(
            sp['az_main_rad'], sp['null_az_rads'], N, d_spacing, az_grid)
        gain_db   = 10.0 * np.log10(np.maximum(gain_lin, 1e-12))

        rx_dbm    = tx_dbm - pl_db + gain_db
        rx_mw[i]  = 10.0 ** (rx_dbm / 10.0)

    # Best-serving AP: max received power
    total_mw  = np.sum(rx_mw, axis=0)
    best_idx  = np.argmax(rx_mw, axis=0)
    signal_mw = rx_mw[best_idx, np.arange(n_pts)]
    interf_mw = total_mw - signal_mw
    sinr_lin  = signal_mw / (interf_mw + noise_mw)
    sinr_db   = 10.0 * np.log10(np.maximum(sinr_lin, 1e-10))
    return sinr_db.reshape(xx.shape)


# ---------------------------------------------------------------------------
# Overlay map — beams superposed on the floor plan
# ---------------------------------------------------------------------------

def _fallback_scale_radius(nodes, specs):
    """Fallback beam-shape radius when an AP has no associated STA."""
    aps = [sp['node'] for sp in specs]
    if len(aps) >= 2:
        dists = []
        for i in range(len(aps)):
            for j in range(i + 1, len(aps)):
                dists.append(math.hypot(aps[i]['x'] - aps[j]['x'],
                                        aps[i]['y'] - aps[j]['y']))
        return 0.35 * min(dists)
    xs = [n['x'] for n in nodes]
    ys = [n['y'] for n in nodes]
    diag = math.hypot(max(xs) - min(xs) + 1e-3, max(ys) - min(ys) + 1e-3)
    return 0.30 * diag


def plot_beam_overlay(ax, nodes, specs, db_floor=-40.0,
                      noise_dbm=-95.0, alpha=3.5, grid_res=200,
                      sinr_vmin=-5.0, sinr_vmax=35.0):
    """Draw beam gain shapes overlaid on an SINR heatmap.

    Beam radius scaling
    -------------------
    Each AP's beam shape is scaled so the 0 dB contour touches its associated
    STA (scale_r = AP→STA distance).  If no STA exists, falls back to a
    fraction of the inter-AP distance.

    SINR heatmap
    ------------
    For every grid point the received power from each BF-enabled AP is
    computed using a log-distance path-loss model (exponent alpha) plus the
    AP's beam gain toward that point.  The displayed SINR is for the
    best-serving AP (max received power), with all others as interference.
    """
    wlan_codes = sorted(set(n['wlan'] for n in nodes))
    color_map  = {w: COLORS[i % len(COLORS)] for i, w in enumerate(wlan_codes)}

    # ------------------------------------------------------------------ #
    # 1. SINR heatmap                                                      #
    # ------------------------------------------------------------------ #
    all_x = [n['x'] for n in nodes]
    all_y = [n['y'] for n in nodes]
    margin = max((max(all_x) - min(all_x)) * 0.20,
                 (max(all_y) - min(all_y)) * 0.20,
                 0.5)
    x_lo, x_hi = min(all_x) - margin, max(all_x) + margin
    y_lo, y_hi = min(all_y) - margin, max(all_y) + margin

    gx_1d = np.linspace(x_lo, x_hi, grid_res)
    gy_1d = np.linspace(y_lo, y_hi, grid_res)
    xx, yy = np.meshgrid(gx_1d, gy_1d)

    sinr = compute_sinr_grid(specs, xx, yy, noise_dbm, alpha)

    im = ax.imshow(sinr,
                   origin='lower',
                   extent=[x_lo, x_hi, y_lo, y_hi],
                   cmap='RdYlGn',
                   vmin=sinr_vmin, vmax=sinr_vmax,
                   aspect='equal',
                   interpolation='bilinear',
                   zorder=0)

    cbar = plt.colorbar(im, ax=ax, fraction=0.035, pad=0.02)
    cbar.set_label('SINR (dB)', fontsize=8)
    cbar.ax.tick_params(labelsize=7)

    # ------------------------------------------------------------------ #
    # 2. Beam shapes (scaled to AP→STA distance)                          #
    # ------------------------------------------------------------------ #
    fallback_r = _fallback_scale_radius(nodes, specs)
    n_az = 3600
    az_arr = np.linspace(0, 2.0 * math.pi, n_az, endpoint=False)

    for i, sp in enumerate(specs):
        ap      = sp['node']
        N       = max(ap['bf_N'], 1)
        d_sp    = ap['bf_d']
        color   = color_map[ap['wlan']]
        scale_r = sp['sta_dist'] if sp['sta_dist'] else fallback_r

        gain     = beam_gain_at_azimuths(sp['az_main_rad'], sp['null_az_rads'],
                                         N, d_sp, az_arr)
        gain_db  = 10.0 * np.log10(np.maximum(gain, 10.0 ** (db_floor / 10.0)))
        r_vals   = scale_r * (gain_db - db_floor) / (-db_floor)

        x_shape = ap['x'] + r_vals * np.cos(az_arr)
        y_shape = ap['y'] + r_vals * np.sin(az_arr)

        ax.fill(x_shape, y_shape, color=color, alpha=0.22, zorder=1)
        ax.plot(np.append(x_shape, x_shape[0]),
                np.append(y_shape, y_shape[0]),
                color=color, lw=1.3, alpha=0.9, zorder=2)

        # Dotted circle: 0 dB reference = STA distance
        theta_c = np.linspace(0, 2.0 * math.pi, 360)
        ax.plot(ap['x'] + scale_r * np.cos(theta_c),
                ap['y'] + scale_r * np.sin(theta_c),
                color=color, lw=0.6, ls=':', alpha=0.5, zorder=2)

    # ------------------------------------------------------------------ #
    # 3. AP→STA links                                                     #
    # ------------------------------------------------------------------ #
    sta_by_wlan = {n['wlan']: n for n in nodes if n['type'] == 1}
    for sp in specs:
        ap  = sp['node']
        sta = sta_by_wlan.get(ap['wlan'])
        if sta is not None:
            ax.plot([ap['x'], sta['x']], [ap['y'], sta['y']],
                    '-', color=color_map[ap['wlan']], lw=1.2,
                    alpha=0.7, zorder=3)

    # ------------------------------------------------------------------ #
    # 4. Null-direction arrows                                            #
    # ------------------------------------------------------------------ #
    for sp in specs:
        ap      = sp['node']
        scale_r = sp['sta_dist'] if sp['sta_dist'] else fallback_r
        for null_az in sp['null_az_rads']:
            nx = ap['x'] + scale_r * 1.2 * math.cos(null_az)
            ny = ap['y'] + scale_r * 1.2 * math.sin(null_az)
            ax.annotate('', xy=(nx, ny), xytext=(ap['x'], ap['y']),
                        arrowprops=dict(arrowstyle='->', color='white',
                                        lw=1.2, linestyle='dashed'),
                        zorder=4)

    # ------------------------------------------------------------------ #
    # 5. Node markers + labels                                            #
    # ------------------------------------------------------------------ #
    for n in nodes:
        color  = color_map[n['wlan']]
        marker = 's' if n['type'] == 0 else 'o'
        ms     = 100 if n['type'] == 0 else 60
        ax.scatter(n['x'], n['y'], c=color, marker=marker, s=ms,
                   edgecolors='k', linewidths=0.8, zorder=5)
        ax.annotate(n['code'], (n['x'], n['y']),
                    textcoords='offset points', xytext=(5, 4),
                    fontsize=7, color='white', fontweight='bold',
                    zorder=6)

    # ------------------------------------------------------------------ #
    # 6. Legend + formatting                                              #
    # ------------------------------------------------------------------ #
    handles = [mpatches.Patch(color=color_map[w], label='WLAN ' + w)
               for w in wlan_codes]
    handles += [
        plt.Line2D([0], [0], marker='s', color='w', markerfacecolor='gray',
                   markeredgecolor='k', markersize=8, label='AP'),
        plt.Line2D([0], [0], marker='o', color='w', markerfacecolor='gray',
                   markeredgecolor='k', markersize=8, label='STA'),
        mpatches.Patch(color='gray', alpha=0.35, label='Beam shape'),
        plt.Line2D([0], [0], ls=':', color='gray', label='0 dB = STA dist'),
        plt.Line2D([0], [0], ls='--', color='white', label='Null direction'),
    ]
    ax.legend(handles=handles, fontsize=7, loc='upper left',
              facecolor='#222222', labelcolor='white', framealpha=0.75)
    ax.set_aspect('equal')
    ax.set_xlabel('x (m)')
    ax.set_ylabel('y (m)')
    ax.text(0.01, 0.01,
            u'PL exp \u03b1={:.1f} | Noise={:.0f} dBm | Floor={:.0f} dB'.format(
                alpha, noise_dbm, db_floor),
            transform=ax.transAxes, fontsize=6.5, va='bottom', color='white',
            bbox=dict(boxstyle='round,pad=0.25', fc='black', alpha=0.55))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description='Plot ULA beam patterns from Komondor node CSV.')
    parser.add_argument('--nodes', required=True, help='Node CSV file')
    parser.add_argument('--mapc',  default=None,  help='MAPC config CSV (optional)')
    parser.add_argument('--out',         default=None, help='Save Fig 1 (polars) to file')
    parser.add_argument('--out-overlay', default=None, help='Save Fig 2 (map overlay) to file')
    parser.add_argument('--db-floor', type=float, default=-40.0,
                        help='Beam gain floor for display [dB], default -40')
    parser.add_argument('--noise-dbm', type=float, default=-95.0,
                        help='Thermal noise power [dBm], default -95')
    parser.add_argument('--path-loss-exp', type=float, default=3.5,
                        help='Path-loss exponent alpha, default 3.5 (indoor)')
    parser.add_argument('--grid-res', type=int, default=200,
                        help='SINR grid resolution (NxN), default 200')
    parser.add_argument('--sinr-min', type=float, default=-5.0,
                        help='SINR colormap minimum [dB], default -5')
    parser.add_argument('--sinr-max', type=float, default=35.0,
                        help='SINR colormap maximum [dB], default 35')
    args = parser.parse_args()

    nodes = parse_nodes(args.nodes)
    mapc_groups = parse_mapc(args.mapc) if args.mapc else []

    specs = build_ap_beam_specs(nodes, mapc_groups)

    if not specs:
        print('No BF-enabled AP nodes found in {}'.format(args.nodes))
        sys.exit(0)

    title_base = 'Beam patterns — ' + args.nodes
    if args.mapc:
        title_base += '  |  MAPC: ' + args.mapc

    # ---- Figure 1: floor plan + individual polar plots ----
    n_specs = len(specs)
    n_cols  = 1 + n_specs
    fig1    = plt.figure(figsize=(4.5 * n_cols, 5))
    gs      = fig1.add_gridspec(1, n_cols, wspace=0.35)

    ax_floor = fig1.add_subplot(gs[0, 0])
    plot_floor_plan(ax_floor, nodes, specs)

    for i, sp in enumerate(specs):
        color = COLORS[i % len(COLORS)]
        ax_p  = fig1.add_subplot(gs[0, i + 1], projection='polar')
        plot_polar_beam(ax_p, sp, color, db_floor=args.db_floor)
        title  = 'AP {} — main {:.1f}°'.format(
            sp['node']['code'], math.degrees(sp['az_main_rad']))
        if sp['null_labels']:
            title += '\nnulls → {}'.format(', '.join(sp['null_labels']))
        ax_p.set_title(title, fontsize=8, pad=10)
        ax_p.legend(fontsize=7, loc='upper right', bbox_to_anchor=(1.35, 1.1))

    fig1.suptitle(title_base, fontsize=9)

    if args.out:
        fig1.savefig(args.out, dpi=150, bbox_inches='tight')
        print('Fig 1 saved to', args.out)

    # ---- Figure 2: beams superposed on the map ----
    fig2, ax2 = plt.subplots(figsize=(7, 6))
    plot_beam_overlay(ax2, nodes, specs,
                      db_floor=args.db_floor,
                      noise_dbm=args.noise_dbm,
                      alpha=args.path_loss_exp,
                      grid_res=args.grid_res,
                      sinr_vmin=args.sinr_min,
                      sinr_vmax=args.sinr_max)
    ax2.set_title(title_base + '\nBeam overlay + SINR heatmap', fontsize=9)
    fig2.tight_layout()

    if args.out_overlay:
        fig2.savefig(args.out_overlay, dpi=150, bbox_inches='tight')
        print('Fig 2 saved to', args.out_overlay)

    if not args.out or not args.out_overlay:
        plt.show()


if __name__ == '__main__':
    main()

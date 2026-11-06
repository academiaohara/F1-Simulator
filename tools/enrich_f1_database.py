#!/usr/bin/env python3
"""Fill gameplay-critical 2026 circuit facts missing from the supplied DB."""
from __future__ import annotations

import sqlite3
import sys


# length km, race laps, corners
CIRCUITS = {
    "albert_park": (5.278, 58, 14), "americas": (5.513, 56, 20),
    "bahrain": (5.412, 57, 15), "baku": (6.003, 51, 20),
    "barcelona": (4.657, 66, 14), "hungaroring": (4.381, 70, 14),
    "imola": (4.909, 63, 19), "interlagos": (4.309, 71, 15),
    "jeddah": (6.174, 50, 27), "las_vegas": (6.201, 50, 17),
    "madring": (5.474, 57, 22), "marina_bay": (4.940, 62, 19),
    "miami": (5.412, 57, 19), "monaco": (3.337, 78, 19),
    "monza": (5.793, 53, 11), "red_bull_ring": (4.318, 71, 10),
    "rodriguez": (4.304, 71, 17), "shanghai": (5.451, 56, 16),
    "silverstone": (5.891, 52, 18), "spa": (7.004, 44, 19),
    "suzuka": (5.807, 53, 18), "villeneuve": (4.361, 70, 14),
    "yas_marina": (5.281, 58, 16), "zandvoort": (4.259, 72, 14),
}


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: enrich_f1_database.py DATABASE", file=sys.stderr)
        return 2
    db = sqlite3.connect(sys.argv[1])
    with db:
        for circuit_id, (length, laps, corners) in CIRCUITS.items():
            result = db.execute(
                "update circuits set length_km=?, laps=?, corners=?, map_asset=? where id=?",
                (length, laps, corners, f"embedded:{circuit_id}", circuit_id),
            )
            if result.rowcount != 1:
                raise RuntimeError(f"circuit not found: {circuit_id}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Generate dependency-free C tables from the editable F1 SQLite database."""
from __future__ import annotations

import sqlite3
import sys
from pathlib import Path


FALLBACK_TRACK = ((80, 520), (150, 250), (370, 90), (700, 110),
                  (920, 340), (870, 700), (610, 900), (260, 840),
                  (80, 520))


def cstr(value: object) -> str:
    text = "" if value is None else str(value)
    return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'


def color(value: object) -> int:
    text = str(value or "#000000").lstrip("#")
    return int(text, 16)


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: export_f1_data.py DATABASE OUTPUT", file=sys.stderr)
        return 2
    db_path, output = map(Path, sys.argv[1:])
    db = sqlite3.connect(db_path)
    db.row_factory = sqlite3.Row

    teams = list(db.execute("select * from teams order by id"))
    drivers = list(db.execute("""
        select d.*, c.team_id from drivers d
        join contracts c on c.driver_id=d.id and c.season_id=2026
        order by c.team_id, c.role
    """))
    circuits = list(db.execute("select * from circuits order by id"))
    rounds = list(db.execute("select * from rounds order by round_number"))

    lines = ['#include "f1_data.h"', '']
    lines.append("const F1TeamDef F1_TEAMS[] = {")
    for r in teams:
        lines.append("    {%s,%s,%s,%s,%s,%s,0x%06Xu,0x%06Xu,%d}," % (
            cstr(r['id']), cstr(r['name']), cstr(r['code']), cstr(r['country']),
            cstr(r['base']), cstr(r['power_unit_id']), color(r['color_primary']),
            color(r['color_secondary']), r['reputation']))
    lines.append("};")
    lines.append("const int F1_TEAM_COUNT = (int)(sizeof F1_TEAMS / sizeof F1_TEAMS[0]);\n")

    lines.append("const F1DriverDef F1_DRIVERS[] = {")
    for r in drivers:
        lines.append("    {%s,%s,%s,%s,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d}," % (
            cstr(r['id']), cstr(r['first_name']), cstr(r['last_name']), cstr(r['code']),
            cstr(r['nationality']), cstr(r['team_id']), r['number'], r['pace'],
            r['qualifying'], r['racecraft'], r['consistency'], r['tyre_management'],
            r['wet_skill'], r['potential']))
    lines.append("};")
    lines.append("const int F1_DRIVER_COUNT = (int)(sizeof F1_DRIVERS / sizeof F1_DRIVERS[0]);\n")

    points = ",".join("{%d,%d}" % p for p in FALLBACK_TRACK)
    lines.append("const F1CircuitDef F1_CIRCUITS[] = {")
    for r in circuits:
        lines.append("    {%s,%s,%s,%.3ff,%.2ff,%.3ff,%.3ff,%d,%d,%d,%d,{%s}}," % (
            cstr(r['id']), cstr(r['name']), cstr(r['country']), r['length_km'] or 5.0,
            r['pit_loss_seconds'], r['safety_car_probability'], r['rain_probability'],
            r['laps'] or 55, r['overtaking'], r['tyre_degradation'], len(FALLBACK_TRACK), points))
    lines.append("};")
    lines.append("const int F1_CIRCUIT_COUNT = (int)(sizeof F1_CIRCUITS / sizeof F1_CIRCUITS[0]);\n")

    lines.append("const F1RoundDef F1_ROUNDS[] = {")
    for r in rounds:
        lines.append("    {%s,%s,%s,%d,%d}," % (cstr(r['id']), cstr(r['circuit_id']),
                     cstr(r['name']), r['round_number'], 1 if r['format'] == 'SPRINT' else 0))
    lines.append("};")
    lines.append("const int F1_ROUND_COUNT = (int)(sizeof F1_ROUNDS / sizeof F1_ROUNDS[0]);\n")

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("\n".join(lines), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

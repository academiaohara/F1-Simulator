#!/usr/bin/env python3
"""Normalize MIT-licensed circuit GeoJSON into compact game coordinates."""
from __future__ import annotations

import json
import math
import sys
from pathlib import Path


FILES = {
    "albert_park": "au-1953.geojson", "americas": "us-2012.geojson",
    "bahrain": "bh-2002.geojson", "baku": "az-2016.geojson",
    "barcelona": "es-1991.geojson", "hungaroring": "hu-1986.geojson",
    "imola": "it-1953.geojson", "interlagos": "br-1940.geojson",
    "jeddah": "sa-2021.geojson", "las_vegas": "us-2023.geojson",
    "madring": "es-2026.geojson", "marina_bay": "sg-2008.geojson",
    "miami": "us-2022.geojson", "monaco": "mc-1929.geojson",
    "monza": "it-1922.geojson", "red_bull_ring": "at-1969.geojson",
    "rodriguez": "mx-1962.geojson", "shanghai": "cn-2004.geojson",
    "silverstone": "gb-1948.geojson", "spa": "be-1925.geojson",
    "suzuka": "jp-1962.geojson", "villeneuve": "ca-1978.geojson",
    "yas_marina": "ae-2009.geojson", "zandvoort": "nl-1948.geojson",
}


def normalize(coords: list[list[float]], count: int = 48) -> list[list[int]]:
    indices = [round(i * (len(coords) - 1) / (count - 1)) for i in range(count)]
    sampled = [coords[i] for i in indices]
    mean_lat = sum(p[1] for p in sampled) / len(sampled)
    cos_lat = math.cos(math.radians(mean_lat))
    xs = [p[0] * cos_lat for p in sampled]
    ys = [-p[1] for p in sampled]
    min_x, max_x, min_y, max_y = min(xs), max(xs), min(ys), max(ys)
    span_x, span_y = max_x - min_x, max_y - min_y
    scale = 900.0 / max(span_x, span_y)
    offset_x = (1000.0 - span_x * scale) / 2.0
    offset_y = (1000.0 - span_y * scale) / 2.0
    return [[round(offset_x + (x - min_x) * scale),
             round(offset_y + (y - min_y) * scale)] for x, y in zip(xs, ys)]


def main() -> int:
    if len(sys.argv) != 3:
        print("usage: import_track_shapes.py GEOJSON_DIR OUTPUT", file=sys.stderr)
        return 2
    source, output = map(Path, sys.argv[1:])
    tracks = {}
    for circuit_id, filename in FILES.items():
        data = json.loads((source / filename).read_text(encoding="utf-8"))
        feature = next(f for f in data["features"] if f["geometry"]["type"] == "LineString")
        tracks[circuit_id] = normalize(feature["geometry"]["coordinates"])
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(tracks, separators=(",", ":")), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

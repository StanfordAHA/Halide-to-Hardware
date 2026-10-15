"""hw_support at the CGRA data width (the lake spec's data_width, read from the lake
collateral `aha map` sets in LAKE_COLLATERAL_JSON_MEM; 16 without one).

One .raw / .txt element is one CGRA word: 2 bytes at 16 bits, 4 at 32, 8 at 64,
big-endian in .raw (the testbench's order).
"""
import json
import os
import subprocess
import sys
from pathlib import Path

import numpy as np
import pytest

HW = Path(__file__).resolve().parent.parent
WIDTHS = (8, 16, 32, 64)


def _env(tmp_path, width):
    env = dict(os.environ)
    env.pop("LAKE_COLLATERAL_JSON_MEM", None)
    if width is not None:
        p = tmp_path / "lake_collateral_mem.json"
        p.write_text(json.dumps({"data_width": width}))
        env["LAKE_COLLATERAL_JSON_MEM"] = str(p)
    return env


@pytest.mark.parametrize("width", (None,) + WIDTHS)
def test_raw2txt_reports_the_collateral_width(tmp_path, width):
    out = subprocess.run([sys.executable, str(HW / "raw2txt.py"), "--data-width"], env=_env(tmp_path, width),
                         capture_output=True, text=True, check=True)
    assert out.stdout.strip() == str(width or 16)


@pytest.mark.parametrize("width", (16, 32, 64))
def test_raw2txt_one_word_per_element(tmp_path, width):
    words = [0, 1, (1 << width) - 1, 0x0102030405060708 & ((1 << width) - 1)]
    raw = tmp_path / "x.raw"
    np.array(words, dtype=f">u{width // 8}").tofile(raw)  # .raw is big-endian CGRA words
    subprocess.run([sys.executable, str(HW / "raw2txt.py"), str(raw)], env=_env(tmp_path, width), check=True,
                   capture_output=True)
    hexes = (tmp_path / "x.txt").read_text().split()
    assert [int(h, 16) for h in hexes] == words
    assert {len(h) for h in hexes} == {width // 4}


def test_raw2txt_16_bits_unchanged(tmp_path):
    # the pre-width code: np.fromfile(..., uint16).byteswap(), 4 hex digits per value
    raw = tmp_path / "x.raw"
    np.array([0x1234, 0xabcd, 7], dtype=">u2").tofile(raw)
    subprocess.run([sys.executable, str(HW / "raw2txt.py"), str(raw)], env=_env(tmp_path, None), check=True,
                   capture_output=True)
    ref = [f"{int(v):04x}" for v in np.fromfile(raw, dtype=np.uint16).byteswap()]
    assert (tmp_path / "x.txt").read_text().split() == ref


@pytest.mark.parametrize("width", (16, 32, 64))
def test_make_leraw_to_raw_swaps_each_word(tmp_path, width):
    # hardware_targets.mk's $(BIN)/%.raw: $(BIN)/%.leraw rule (dd conv=swab only at 16)
    words = np.array([0x0102030405060708, 0x1122334455667788, 3], dtype=np.uint64) & np.uint64((1 << width) - 1)
    words = words.astype(f"<u{width // 8}")
    bin_dir = tmp_path / "bin"
    bin_dir.mkdir()
    words.tofile(bin_dir / "x.leraw")
    subprocess.run(["make", "-s", "-f", str(HW / "hardware_targets.mk"), f"BIN={bin_dir}", f"HWSUPPORT={HW}",
                    str(bin_dir / "x.raw")], cwd=tmp_path, env=_env(tmp_path, width), check=True,
                   capture_output=True)
    assert list(np.fromfile(bin_dir / "x.raw", dtype=f">u{width // 8}")) == list(words)

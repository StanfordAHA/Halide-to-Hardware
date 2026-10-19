"""parse_design_meta.py's bitstream choice: with several *.bs in bin/ (a stale one from an
earlier run, or one carried over by copying an app directory), design_meta.json must name the
one garnet.py wrote last, not whichever os.listdir returns first."""
import os
import sys
from pathlib import Path

import pytest

HW = Path(__file__).resolve().parent.parent


def _module():
    pytest.importorskip("voyager.scripts.aha_flow.glb_dma_config")
    sys.path.insert(0, str(HW))
    import parse_design_meta
    return parse_design_meta


def test_newest_bitstream(tmp_path):
    pdm = _module()
    assert pdm.bitstream_file(tmp_path) is None
    (tmp_path / "design.place").write_text("")
    for i, name in enumerate(("zz_new.bs", "aa_old.bs", "mm_older.bs")):
        p = tmp_path / name
        p.write_text("")
        os.utime(p, (1000 - 100 * i, 1000 - 100 * i))
    assert pdm.bitstream_file(tmp_path) == "zz_new.bs"
    os.utime(tmp_path / "aa_old.bs", (2000, 2000))
    assert pdm.bitstream_file(tmp_path) == "aa_old.bs"

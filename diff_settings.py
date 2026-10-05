# asm-differ settings: tools/venv/bin/python tools/ext/asm-differ/diff.py -mw <function>
# Diffs the built EXE against the original (needs a linked build/, i.e. `ninja` once).
def apply(config, args):
    config["arch"] = "mipsel"
    config["baseimg"] = "extracted/disc/SLES_039.36"
    config["myimg"] = "build/SLES_039.36"
    config["mapfile"] = "build/main/SLES_039.36.map"
    config["source_directories"] = ["src", "include", "asm"]
    config["objdump_executable"] = "tools/binutils/bin/mipsel-linux-gnu-objdump"
    config["make_command"] = ["ninja"]

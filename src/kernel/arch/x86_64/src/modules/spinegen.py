# import os
# import sys
# import pathlib
# import re
# print(os.getcwd())
# print(sys.argv)
# # Generate argv[-1]
# output = pathlib.Path(sys.argv[-1])
# if output.exists():
#     raise Exception("Refusing to overwrite")

# incldir = pathlib.Path(sys.argv[-2])
# files = list(incldir.rglob("*.hpp"))
# # print(files)

# names = []
# for header in files:
#     lines = [a.strip() for a in header.read_text().splitlines()]
#     for line in lines:
#         if line.startswith("EXPORT"):
#             # we should have EXPORT void func() so
#             name_full = line.split()[2]
#             name = re.split("[^a-zA-Z0-9]", name_full)[0]
#             names.append(name)

# # print(names)
# # generate output files
# file = """#include <modules/spine.hpp>
# """
# for name in names:
#     file += f"extern \"C\" void* {name};\n"
# file += """
# namespace modules {
# void initSpine() {
# """
# for name in names:
#     file += f"\tspineSet(\"{name}\", {name});\n"
# file += """}
# }"""

# # print(file)

# with open(output, "wt+") as f:
#     f.write(file)

import sys
from pathlib import Path

f = [(int(a, 16), b.strip()) for [a,b] in [line.split(" ") for line in sys.stdin.readlines()]]
# The format is as follows:
# <name> <null byte for end of string> <eight bytes of addr> and repeat
ba = bytearray()
for (addr, name) in f:
    for c in name:
        ba.append(ord(c))
    ba.append(0)
    ba += addr.to_bytes(8, 'little')

out = Path(sys.argv[-1])
if out.exists():
    raise Exception("Refusing to overwrite")
with open(out, "wb+") as file:
    file.write(ba)

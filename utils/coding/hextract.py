#!/usr/bin/env python3
"""
Extract a .h file from a .c file.
Rules:
  - #define lines -> kept as-is
  - #include lines -> kept as-is
  - lines starting with 'const ' -> turned into 'extern const ...' declaration
  - lines starting with 'inline ' -> extract signature up to '{', emit as declaration with ';'
  - everything else -> skipped
"""

import sys
import os
import re

def extract_header(c_path, h_path=None):
    if h_path is None:
        base = os.path.splitext(c_path)[0]
        h_path = base + ".generated.h"

    with open(c_path, 'r') as f:
        lines = f.readlines()

    guard = os.path.basename(h_path).upper().replace('.', '_').replace('-', '_')

    out = []
    out.append(f"#ifndef {guard}\n")
    out.append(f"#define {guard}\n")
    out.append("\n")

    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        # #include and #define -> pass through
        if stripped.startswith('#include') or stripped.startswith('#define'):
            out.append(line)
            i += 1
            continue

        # const global -> extern const declaration
        # e.g. "const point3d BBRIGHT = {1, 0, 0};" -> "extern const point3d BBRIGHT;"
        if stripped.startswith('const '):
            # grab type and name (everything before '=')
            m = re.match(r'const\s+(\w+)\s+(\w+)\s*=', stripped)
            if m:
                typ, name = m.group(1), m.group(2)
                # preserve any trailing comment
                comment = ''
                if '//' in stripped:
                    comment = '  ' + stripped[stripped.index('//'):]
                out.append(f"extern const {typ} {name};{comment}\n")
            i += 1
            continue

        # inline function -> collect until we hit '{', emit signature + ';'
        if stripped.startswith('inline '):
            # accumulate lines until we find the opening brace
            sig_lines = []
            while i < len(lines):
                sig_line = lines[i].rstrip()
                sig_lines.append(sig_line)
                if '{' in sig_line:
                    break
                i += 1
            # join, strip everything from '{' onward
            full_sig = ' '.join(l.strip() for l in sig_lines)
            full_sig = full_sig[:full_sig.index('{')].strip().rstrip()
            out.append(f"{full_sig};\n")
            i += 1
            continue

        i += 1

    out.append("\n")
    out.append(f"#endif // {guard}\n")

    with open(h_path, 'w') as f:
        f.writelines(out)

    print(f"Written: {h_path}")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: extract_header.py <file.c> [output.h]")
        sys.exit(1)
    c_file = sys.argv[1]
    h_file = sys.argv[2] if len(sys.argv) > 2 else None
    extract_header(c_file, h_file)
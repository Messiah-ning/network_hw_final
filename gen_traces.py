#!/usr/bin/env python3
"""Generate trace packets from a ClassBench rules file.

Output format per line (tab-separated):
  src_ip  dst_ip  src_port  dst_port  protocol  flags  result
Only the first 5 fields are used by MultilayerTuple's ReadTraces().
"""

import sys
import random
import struct
import socket
import argparse


def ip_to_int(ip_str):
    return struct.unpack("!I", socket.inet_aton(ip_str))[0]


def parse_rules(rules_file):
    """Parse ClassBench v4 rules file. Returns list of rule dicts."""
    rules = []
    with open(rules_file) as f:
        for line in f:
            line = line.strip()
            if not line or not line.startswith("@"):
                continue
            line = line[1:]  # strip leading @
            # Split by tab, space, slash, colon (same as StrSplit in io.cpp)
            import re
            tokens = re.split(r"[\t /:]", line)
            tokens = [t for t in tokens if t]

            # src_ip / prefix
            src_ip_base = ip_to_int(tokens[0])
            src_prefix = int(tokens[1])
            src_mask = (1 << (32 - src_prefix)) - 1 if src_prefix < 32 else 0
            src_ip_lo = src_ip_base & ~src_mask
            src_ip_hi = src_ip_lo | src_mask

            # dst_ip / prefix
            dst_ip_base = ip_to_int(tokens[2])
            dst_prefix = int(tokens[3])
            dst_mask = (1 << (32 - dst_prefix)) - 1 if dst_prefix < 32 else 0
            dst_ip_lo = dst_ip_base & ~dst_mask
            dst_ip_hi = dst_ip_lo | dst_mask

            # src_port range
            src_port_lo = int(tokens[4])
            src_port_hi = int(tokens[5])

            # dst_port range
            dst_port_lo = int(tokens[6])
            dst_port_hi = int(tokens[7])

            # protocol (value / mask)
            proto_val = int(tokens[8], 0)
            proto_mask = int(tokens[9], 0)
            inv_mask = 255 ^ proto_mask
            proto_lo = proto_val & ~inv_mask
            proto_hi = proto_lo | inv_mask

            rules.append({
                "src_ip": (src_ip_lo, src_ip_hi),
                "dst_ip": (dst_ip_lo, dst_ip_hi),
                "src_port": (src_port_lo, src_port_hi),
                "dst_port": (dst_port_lo, dst_port_hi),
                "protocol": (proto_lo, proto_hi),
            })
    return rules


def generate_traces(rules, traces_per_rule, output_file):
    with open(output_file, "w") as f:
        for rule in rules:
            for _ in range(traces_per_rule):
                src_ip = random.randint(*rule["src_ip"])
                dst_ip = random.randint(*rule["dst_ip"])
                src_port = random.randint(*rule["src_port"])
                dst_port = random.randint(*rule["dst_port"])
                protocol = random.randint(*rule["protocol"])
                f.write(f"{src_ip}\t{dst_ip}\t{src_port}\t{dst_port}\t{protocol}\t0\t0\n")


def main():
    parser = argparse.ArgumentParser(description="Generate traces from ClassBench rules")
    parser.add_argument("rules_file", help="Input rules file (ClassBench v4 format)")
    parser.add_argument("output_file", help="Output traces file")
    parser.add_argument("--traces-per-rule", type=int, default=10,
                        help="Number of trace packets per rule (default: 10)")
    parser.add_argument("--seed", type=int, default=42, help="Random seed")
    args = parser.parse_args()

    random.seed(args.seed)
    print(f"Parsing rules from {args.rules_file} ...")
    rules = parse_rules(args.rules_file)
    print(f"Parsed {len(rules)} rules. Generating {len(rules) * args.traces_per_rule} traces ...")
    generate_traces(rules, args.traces_per_rule, args.output_file)
    print(f"Done. Written to {args.output_file}")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Generate synthetic ClassBench v4 rules by sampling from an existing rule set's distribution."""

import sys
import re
import random
import struct
import socket
import argparse


def ip_to_int(ip_str):
    return struct.unpack("!I", socket.inet_aton(ip_str))[0]


def int_to_ip(n):
    return socket.inet_ntoa(struct.pack("!I", n & 0xFFFFFFFF))


def parse_rules(rules_file):
    rules = []
    with open(rules_file) as f:
        for line in f:
            line = line.strip()
            if not line or not line.startswith("@"):
                continue
            tokens = re.split(r"[\t /:]", line[1:])
            tokens = [t for t in tokens if t]

            src_ip = ip_to_int(tokens[0])
            src_prefix = int(tokens[1])
            dst_ip = ip_to_int(tokens[2])
            dst_prefix = int(tokens[3])
            src_port_lo = int(tokens[4])
            src_port_hi = int(tokens[5])
            dst_port_lo = int(tokens[6])
            dst_port_hi = int(tokens[7])
            proto_val = int(tokens[8], 0)
            proto_mask = int(tokens[9], 0)
            flags_val = tokens[10] if len(tokens) > 10 else "0x0000"
            flags_mask = tokens[11] if len(tokens) > 11 else "0x0000"

            # Normalize src_ip to network address
            src_mask = (1 << (32 - src_prefix)) - 1 if src_prefix < 32 else 0
            src_net = src_ip & ~src_mask

            dst_mask = (1 << (32 - dst_prefix)) - 1 if dst_prefix < 32 else 0
            dst_net = dst_ip & ~dst_mask

            rules.append({
                "src_net": src_net, "src_prefix": src_prefix,
                "dst_net": dst_net, "dst_prefix": dst_prefix,
                "src_port_lo": src_port_lo, "src_port_hi": src_port_hi,
                "dst_port_lo": dst_port_lo, "dst_port_hi": dst_port_hi,
                "proto_val": proto_val, "proto_mask": proto_mask,
                "flags_val": flags_val, "flags_mask": flags_mask,
            })
    return rules


def build_distributions(rules):
    """Extract statistical distributions from existing rules."""
    src_prefixes = [r["src_prefix"] for r in rules]
    dst_prefixes = [r["dst_prefix"] for r in rules]
    src_nets = [r["src_net"] for r in rules]
    dst_nets = [r["dst_net"] for r in rules]

    # Collect (src_port_lo, src_port_hi) pairs and dst port pairs
    src_port_pairs = [(r["src_port_lo"], r["src_port_hi"]) for r in rules]
    dst_port_pairs = [(r["dst_port_lo"], r["dst_port_hi"]) for r in rules]

    # Collect (proto_val, proto_mask) pairs
    proto_pairs = [(r["proto_val"], r["proto_mask"]) for r in rules]
    flags_pairs = [(r["flags_val"], r["flags_mask"]) for r in rules]

    return {
        "src_prefixes": src_prefixes,
        "dst_prefixes": dst_prefixes,
        "src_nets": src_nets,
        "dst_nets": dst_nets,
        "src_port_pairs": src_port_pairs,
        "dst_port_pairs": dst_port_pairs,
        "proto_pairs": proto_pairs,
        "flags_pairs": flags_pairs,
    }


def generate_rules(dist, count, output_file):
    seen = set()
    written = 0
    max_attempts = count * 20

    with open(output_file, "w") as f:
        attempts = 0
        while written < count and attempts < max_attempts:
            attempts += 1

            # Sample src_ip: pick a random base net from existing ones,
            # then randomize within that /prefix block
            src_prefix = random.choice(dist["src_prefixes"])
            src_net_base = random.choice(dist["src_nets"])
            src_mask = (1 << (32 - src_prefix)) - 1 if src_prefix < 32 else 0
            # Vary the network address by randomizing higher bits slightly
            src_net = (src_net_base + random.randint(0, 255) * (src_mask + 1)) & 0xFFFFFFFF & ~src_mask

            dst_prefix = random.choice(dist["dst_prefixes"])
            dst_net_base = random.choice(dist["dst_nets"])
            dst_mask = (1 << (32 - dst_prefix)) - 1 if dst_prefix < 32 else 0
            dst_net = (dst_net_base + random.randint(0, 255) * (dst_mask + 1)) & 0xFFFFFFFF & ~dst_mask

            src_port_lo, src_port_hi = random.choice(dist["src_port_pairs"])
            dst_port_lo, dst_port_hi = random.choice(dist["dst_port_pairs"])
            proto_val, proto_mask = random.choice(dist["proto_pairs"])
            flags_val, flags_mask = random.choice(dist["flags_pairs"])

            key = (src_net, src_prefix, dst_net, dst_prefix,
                   src_port_lo, src_port_hi, dst_port_lo, dst_port_hi,
                   proto_val, proto_mask)
            if key in seen:
                continue
            seen.add(key)

            src_ip_str = int_to_ip(src_net)
            dst_ip_str = int_to_ip(dst_net)

            f.write(
                f"@{src_ip_str}/{src_prefix}\t{dst_ip_str}/{dst_prefix}\t"
                f"{src_port_lo} : {src_port_hi}\t{dst_port_lo} : {dst_port_hi}\t"
                f"{proto_val:#04x}/{proto_mask:#04x}\t{flags_val}/{flags_mask}\n"
            )
            written += 1

    if written < count:
        print(f"Warning: only generated {written}/{count} unique rules after {attempts} attempts",
              file=sys.stderr)
    return written


def main():
    parser = argparse.ArgumentParser(description="Generate synthetic ClassBench v4 rules")
    parser.add_argument("input_rules", help="Existing rules file to sample distribution from")
    parser.add_argument("output_rules", help="Output rules file")
    parser.add_argument("--count", type=int, default=100000, help="Number of rules to generate")
    parser.add_argument("--seed", type=int, default=42, help="Random seed")
    args = parser.parse_args()

    random.seed(args.seed)
    print(f"Parsing {args.input_rules} ...")
    rules = parse_rules(args.input_rules)
    print(f"Parsed {len(rules)} rules. Building distribution ...")
    dist = build_distributions(rules)
    print(f"Generating {args.count} rules ...")
    written = generate_rules(dist, args.count, args.output_rules)
    print(f"Done. Written {written} rules to {args.output_rules}")


if __name__ == "__main__":
    main()

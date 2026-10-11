#!/usr/bin/env python3
"""Compile, verify and benchmark the two scope implementations (Linux)."""

import argparse
import csv
import os
from pathlib import Path
import platform
import statistics
import subprocess
import tempfile
import time


ROOT = Path(__file__).resolve().parents[1]


def write_input(path, commands):
    path.write_text(str(len(commands)) + "\n" + "\n".join(commands) + "\n")


def workloads(directory, count):
    cases = [("public10", ROOT / "data/public10_random_max_valid.in")]
    for name in ("shallow", "deep_outer", "deep_shadow", "deep_missing"):
        commands = ["Declare int x 1", "Declare int y 0", "Declare int z 0"]
        if name != "shallow":
            for depth in range(99):
                commands.append("Indent")
                if name == "deep_shadow":
                    commands.extend(["Declare int x 1", "Declare int y 0", "Declare int z 0"])
                else:
                    commands.append(f"Declare int local_{depth} 0")
        tail = ["Dedent"] * (99 if name != "shallow" else 0) + ["Print x", "Print z"]
        operations = ["Add z x y", "SelfAdd z 0"]
        if name == "deep_missing":
            operations = ["Add z absent y", "SelfAdd absent 0"]
        for index in range(count - len(commands) - len(tail)):
            commands.append(operations[index % len(operations)])
        commands.extend(tail)
        path = directory / (name + ".in")
        write_input(path, commands)
        cases.append((name, path))

    commands = [f"Declare int variable_{index} 0" for index in range(count - 1)]
    commands.append("Print variable_0")
    path = directory / "declarations.in"
    write_input(path, commands)
    cases.append(("declarations", path))

    # Reuse names in sibling scopes; fewer than 100 Indent commands in total.
    commands = []
    per_scope = (count - 2 * 80 - 1) // 80
    for _ in range(80):
        commands.append("Indent")
        commands.extend(f"Declare int variable_{index} 0" for index in range(per_scope))
        commands.append("Dedent")
    commands.append("Print variable_0")
    path = directory / "scope_churn.in"
    write_input(path, commands)
    cases.append(("scope_churn", path))
    return cases


def verify(binaries, cases, directory):
    for path in sorted((ROOT / "data").glob("*.in")):
        expected = path.with_suffix(".out").read_bytes()
        for binary in binaries:
            with path.open("rb") as input_file:
                result = subprocess.run([binary], stdin=input_file, capture_output=True, check=True)
            if result.stdout != expected:
                raise RuntimeError(f"{binary.name}: incorrect output on {path.name}")

    commands = [
        "Dedent", "Declare int x 7", 'Declare string s "a b"',
        "Indent", 'Declare string x "inner"', "Declare int x 9", "Print x",
        'SelfAdd x "!"', "Print x", "Add x x x", "Print x",
        "Add s s x", "Print s", "Indent", "Declare int x 3",
        "SelfAdd x 2", "Add x x x", "Print x", "Dedent", "Print x",
        "Dedent", "Print x", "Print s", "Indent", "Declare bool x 1",
        "Declare int y bad", "Declare int y 4", "Declare int x 10",
        "SelfAdd x bad", "Print x", "Add x x s", "Print x", "Print y",
        "Dedent", "Print y", "Print x", "Indent", "Declare int y 8",
        "Print y", "Dedent", "Print y", "Dedent", "Print x",
    ]
    # Explicit output also checks that invalid operations leave state intact.
    expected = "\n".join([
        "Invalid operation", "Invalid operation", "x:inner", "x:inner!",
        "x:inner!inner!", "s:a binner!inner!", "x:10", "x:inner!inner!",
        "x:7", "s:a binner!inner!", "Invalid operation", "Invalid operation",
        "Invalid operation", "x:10", "Invalid operation", "x:10", "y:4",
        "Invalid operation", "x:7", "y:8", "Invalid operation",
        "Invalid operation", "x:7", "",
    ]).encode()
    path = directory / "regression.in"
    write_input(path, commands)
    for binary in binaries:
        result = subprocess.run([binary], input=path.read_bytes(), capture_output=True, check=True)
        if result.stdout != expected:
            raise RuntimeError(f"{binary.name}: binding regression failed\n{result.stdout.decode()}")

    # Check the complete output of every timed workload before discarding output during timing.
    for name, path in cases:
        outputs = []
        for binary in binaries:
            output = directory / f"{name}-{binary.name}.out"
            with path.open("rb") as input_file, output.open("wb") as output_file:
                subprocess.run([binary], stdin=input_file, stdout=output_file, check=True)
            outputs.append(output)
        if outputs[0].read_bytes() != outputs[1].read_bytes():
            raise RuntimeError(f"Workload outputs differ: {name}")


def measure(binary, path, directory):
    usage = directory / "usage.txt"
    with path.open("rb") as input_file:
        start = time.perf_counter()
        subprocess.run(
            ["/usr/bin/time", "-f", "%M", "-o", str(usage), str(binary)],
            stdin=input_file, stdout=subprocess.DEVNULL, check=True,
        )
        elapsed = time.perf_counter() - start
    return elapsed * 1000, int(usage.read_text()) / 1024


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repeats", type=int, default=7)
    parser.add_argument("--commands", type=int, default=999999)
    parser.add_argument("--csv", type=Path, help="Save every measurement, including peak RSS")
    args = parser.parse_args()
    if args.repeats < 1 or not 1000 <= args.commands < 1000000:
        parser.error("repeats must be positive; commands must be in [1000, 999999]")
    compiler = os.environ.get("CXX", "g++")
    print(subprocess.check_output([compiler, "--version"], text=True).splitlines()[0], flush=True)
    print(f"{platform.platform()}; C++17 -O2; {args.repeats} repetitions", flush=True)
    rows = []
    with tempfile.TemporaryDirectory(prefix="scope-comparison-") as temporary:
        directory = Path(temporary)
        binaries = [directory / "scope1", directory / "scope2"]
        for binary in binaries:
            subprocess.run([compiler, "-std=c++17", "-O2", str(ROOT / "src" / (binary.name + ".cpp")),
                            "-o", str(binary)], check=True)
        cases = workloads(directory, args.commands)
        verify(binaries, cases, directory)
        print("PASS: 10 public cases, binding regression, and all workload output comparisons", flush=True)
        print("| workload | commands | scope1 ms | scope2 ms | speedup | scope1 MiB | scope2 MiB |", flush=True)
        print("|---|---:|---:|---:|---:|---:|---:|", flush=True)
        for name, path in cases:
            samples = {binary.name: [] for binary in binaries}
            with path.open() as input_file:
                command_count = int(input_file.readline())
            for repetition in range(args.repeats):
                # Alternate execution order to reduce systematic bias.
                order = binaries if repetition % 2 == 0 else binaries[::-1]
                for binary in order:
                    elapsed, rss = measure(binary, path, directory)
                    samples[binary.name].append((elapsed, rss))
                    rows.append((name, command_count, binary.name, repetition + 1, elapsed, rss))
            first, second = [
                (statistics.median(value[0] for value in samples[binary.name]),
                 statistics.median(value[1] for value in samples[binary.name]))
                for binary in binaries
            ]
            print(f"| {name} | {command_count} | {first[0]:.2f} | {second[0]:.2f} | "
                  f"{first[0] / second[0]:.2f}x | {first[1]:.2f} | {second[1]:.2f} |", flush=True)
    if args.csv:
        args.csv.parent.mkdir(parents=True, exist_ok=True)
        with args.csv.open("w", newline="") as output:
            writer = csv.writer(output)
            writer.writerow(["workload", "commands", "version", "repetition", "wall_ms", "peak_rss_mib"])
            writer.writerows(rows)


if __name__ == "__main__":
    main()

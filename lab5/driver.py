#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# driver.py - Test correctness of cache simulator and correctness/perf of transpose
#

import subprocess
import re
import sys
import optparse

def computeMissScore(miss, lower, upper, full_score):
    if miss <= lower:
        return full_score
    if miss >= upper:
        return 0
    score = (miss - lower) * 1.0
    rng = (upper - lower) * 1.0
    return round((1 - score / rng) * full_score, 1)

def run_cmd(args):
    # 以 text 模式捕获 stdout（Python 3）
    res = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                         text=True, check=False)
    return res.stdout

def parse_result_line(output, tag):
    # 在输出中找到以 tag 开头的行，并提取所有数字
    for line in output.splitlines():
        if line.startswith(tag):
            return re.findall(r'(\d+)', line)
    return []

def main():
    # Max scores
    maxscore = {
        'csim': 27,
        'transc': 1,
        'trans32': 8,
        'trans64': 8,
        'trans61': 10,
    }

    # Parse args
    p = optparse.OptionParser()
    p.add_option("-A", action="store_true", dest="autograde",
                 help="emit autoresult string for Autolab")
    opts, args = p.parse_args()
    autograde = opts.autograde

    # Part A: test csim correctness
    print("Part A: Testing cache simulator")
    print("Running ./test-csim")
    out = run_cmd(["./test-csim"])

    resultsim = []
    for line in out.splitlines():
        if line.startswith("TEST_CSIM_RESULTS"):
            resultsim = re.findall(r'(\d+)', line)
        else:
            print(line)

    if not resultsim:
        print("Failed to parse TEST_CSIM_RESULTS from test-csim output.")
        print(out)
        sys.exit(1)

    # Part B: test transpose correctness/performance
    print("Part B: Testing transpose function")

    print("Running ./test-trans -M 32 -N 32")
    out32 = run_cmd(["./test-trans", "-M", "32", "-N", "32"])
    result32 = parse_result_line(out32, "TEST_TRANS_RESULTS")

    print("Running ./test-trans -M 64 -N 64")
    out64 = run_cmd(["./test-trans", "-M", "64", "-N", "64"])
    result64 = parse_result_line(out64, "TEST_TRANS_RESULTS")

    print("Running ./test-trans -M 61 -N 67")
    out61 = run_cmd(["./test-trans", "-M", "61", "-N", "67"])
    result61 = parse_result_line(out61, "TEST_TRANS_RESULTS")

    if not (result32 and result64 and result61):
        print("Failed to parse TEST_TRANS_RESULTS from test-trans output.")
        print("32x32 output:\n", out32)
        print("64x64 output:\n", out64)
        print("61x67 output:\n", out61)
        sys.exit(1)

    # Compute scores
    csim_cscore = int(resultsim[0])
    trans_cscore = int(result32[0]) * int(result64[0]) * int(result61[0])
    miss32 = int(result32[1])
    miss64 = int(result64[1])
    miss61 = int(result61[1])

    trans32_score = computeMissScore(miss32, 300, 600, maxscore['trans32']) * int(result32[0])
    trans64_score = computeMissScore(miss64, 1300, 2000, maxscore['trans64']) * int(result64[0])
    trans61_score = computeMissScore(miss61, 2000, 3000, maxscore['trans61']) * int(result61[0])
    total_score = csim_cscore + trans32_score + trans64_score + trans61_score

    # Summary
    print("\nCache Lab summary:")
    print("%-22s%8s%10s%12s" % ("", "Points", "Max pts", "Misses"))
    print("%-22s%8.1f%10d" % ("Csim correctness", csim_cscore, maxscore['csim']))

    misses = str(miss32)
    if miss32 == 2**31 - 1:
        misses = "invalid"
    print("%-22s%8.1f%10d%12s" % ("Trans perf 32x32", trans32_score,
                                  maxscore['trans32'], misses))

    misses = str(miss64)
    if miss64 == 2**31 - 1:
        misses = "invalid"
    print("%-22s%8.1f%10d%12s" % ("Trans perf 64x64", trans64_score,
                                  maxscore['trans64'], misses))

    misses = str(miss61)
    if miss61 == 2**31 - 1:
        misses = "invalid"
    print("%-22s%8.1f%10d%12s" % ("Trans perf 61x67", trans61_score,
                                  maxscore['trans61'], misses))

    print("%22s%8.1f%10d" % ("Total points", total_score,
                             maxscore['csim'] + maxscore['trans32'] +
                             maxscore['trans64'] + maxscore['trans61']))

    if autograde:
        autoresult = "%.1f:%d:%d:%d" % (total_score, miss32, miss64, miss61)
        print("\nAUTORESULT_STRING=%s" % autoresult)

if __name__ == "__main__":
    main()

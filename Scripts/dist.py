#!/bin/python3

import os
import argparse
import shutil as sh
import pathlib as path

def main():
    stapel_sdk = os.environ["STAPEL_SDK"]

    if not stapel_sdk:
        print("ERROR: You must define STAPEL_SDK in your environment to use this script")
        return -1

    parser = argparse.ArgumentParser("dist.py")
    parser.add_argument("--manifest", default="DistManifest.txt")
    parser.add_argument("--sdk", default=stapel_sdk)
    parser.add_argument("-o", "--output", required=True)

    args = parser.parse_args()

    sdk = path.Path(args.sdk)
    manifest = sdk / args.manifest
    output = path.Path(args.output)

    with open(manifest, "r") as manifest:
        lines = manifest.readlines()

        for line in lines:
            line = line.strip()

            if line == "":
                continue

            [src, dest] = line.split("\t")
            src = sdk / src
            dest = output / dest

            print(f"copying {src} -> {dest}")
            dest.parent.mkdir(parents=True, exist_ok=True)

            sh.copy2(src, dest)

    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main())

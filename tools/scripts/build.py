#!/usr/bin/env python3
# Copyright 2025 The Radium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import sys
import os
import subprocess
import json
import hashlib
import platform

_THIS_DIR = os.path.abspath(os.path.dirname(__file__))
_SRC_DIR = os.path.abspath(os.path.join(_THIS_DIR, "..", "..", ".."))


def calc_md5(content):
    md5obj = hashlib.md5()
    md5obj.update(str.encode(content))
    fhash = md5obj.hexdigest()
    return fhash


def calc_file_md5(filepath):
    with open(filepath, 'r') as f:
        return calc_md5(f.read())


def get_plarform_dir(is_android: bool = False):
    if platform.system().lower() == "windows":
        return "win"
    if platform.system().lower() == "linux":
        return "android" if is_android else "linux"
    if platform.system().lower() == "darwin":
        return "mac"

    assert "Unsupported platform. Please contact liuwd8@foxmail.com"


def write_file(filepath, content):
    with open(filepath, 'w') as f:
        f.write(content)


class BuildProcess():

    def __init__(self, build_dir, target_cpu):
        self.build_dir = build_dir

        if not os.path.exists(self.build_dir):
            os.makedirs(self.build_dir)

        self.target_cpu = target_cpu

    def get_extra_args(self):
        args = '\n'
        status = subprocess.getstatusoutput('ccache --version')
        if status[0] == 0:
            args += 'cc_wrapper = "ccache"\n'

        if self.target_cpu == '' or self.target_cpu == None:
            return args

        args += f'target_cpu = "{self.target_cpu}"\n'
        return args

    def _gen(self, cmd):
        print("gn gen ......")
        try:
            out = subprocess.run(cmd)
            out.check_returncode()
        except:
            exit(1)

    def _build(self, cmd):
        print("build radium ...... " + str(cmd))

        try:
            out = subprocess.run(cmd)
            out.check_returncode()
            print("build succeed!")
        except:
            print("build failed!!!")
            exit(1)


class BuildProcessPosix(BuildProcess):

    def __init__(self, build_dir, target_cpu):
        super().__init__(build_dir, target_cpu)

    def gen(self):
        cmd = ['gn', 'gen', self.build_dir]
        self._gen(cmd)

    def build(self, target):
        cmd = ['autoninja', '-C', self.build_dir]
        cmd.extend(target)

        super()._build(cmd)


class BuildProcessWin(BuildProcess):

    def __init__(self, build_dir, target_cpu):
        super().__init__(build_dir, target_cpu)

    def gen(self):
        cmd = ['gn.bat', 'gen', self.build_dir]
        self._gen(cmd)

    def build(self, target):
        cmd = ['autoninja.bat', '-C', self.build_dir]
        cmd.extend(target)

        super()._build(cmd)


def main():
    parser = argparse.ArgumentParser(description='Build Radium')
    parser.add_argument('--release', action='store_true', help='build release')
    parser.add_argument('--android', action='store_true', help='build release')
    parser.add_argument('--target_cpu', type=str, help='build release')
    parser.add_argument('output_dir', type=str)
    args = parser.parse_args()

    os.chdir(_SRC_DIR)

    if platform.system().lower() == 'windows':
        build_process = BuildProcessWin(args)
    else:
        build_process = BuildProcessPosix(args.output_dir, args.target_cpu)

    gn_args_file = os.path.join(_SRC_DIR, "radium", "build",
                                "release" if args.release else "debug",
                                get_plarform_dir(args.android), 'args.gn')
    origin_args = ''
    with open(gn_args_file, 'r', encoding='utf-8') as f:
        origin_args = f.read()

    origin_args += build_process.get_extra_args()

    md5_origin = calc_md5(origin_args)
    target_args_file = os.path.join(args.output_dir, 'args.gn')
    md5_now = ''
    if os.path.exists(target_args_file):
        md5_now = calc_file_md5(target_args_file)

    regen = False
    if md5_now != md5_origin:
        print('args change')
        write_file(target_args_file, origin_args)
        regen = True
    elif not os.path.exists(os.path.join(args.output_dir, 'build.ninja')):
        print('build.ninja not exist')
        regen = True
    else:
        print('args not change')

    if regen:
        build_process.gen()

    build_process.build(['radium'])


if __name__ == '__main__':
    main()

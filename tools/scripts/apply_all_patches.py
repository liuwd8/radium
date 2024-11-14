# Copyright 2014 The Radium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import sys
import os
import subprocess
import json

def walk_dir(patch_dir):
  paths = []
  for root, _, files in os.walk(patch_dir):
    for file in files:
      paths.append(os.path.join(root, file))

  return paths


def run_command(cmd):
  ret = subprocess.call(cmd)
  if ret != 0:
    if ret <= -100:
      # Windows error codes such as 0xC0000005 and 0xC0000409 are much easier to
      # recognize and differentiate in hex. In order to print them as unsigned
      # hex we need to add 4 Gig to them.
      print('%s failed with exit code 0x%08X' % (sys.argv[1], ret + (1 << 32)))
    else:
      print('%s failed with exit code %d' % (sys.argv[1], ret))

def main():
  parser = argparse.ArgumentParser(description='Apply Electron patches')
  parser.add_argument('--revision', help='chromium revision')
  parser.add_argument('patch_dir',
                      help='patches\' config(s) in the JSON format')
  args =  parser.parse_args()
  patch_dir = os.path.abspath(args.patch_dir)

  mtime = {}
  out_file = os.path.abspath('.radium_mtime.json')
  if os.path.exists(out_file):
    with open(out_file, mode="r", encoding='utf-8') as f:
      mtime = json.load(f)

  os.chdir('src')
  paths = walk_dir(patch_dir)

  need_repatch = False
  for i in paths:
    if not i in mtime:
      need_repatch = True
      break

    if mtime[i] != os.path.getmtime(i):
      need_repatch = True
      break

  if not need_repatch:
    return

  run_command(['git', 'reset', '--hard', args.revision])
  for i in paths:
    run_command(['git', 'am', i])
    mtime[i] = os.path.getmtime(i)

  with open(out_file, mode="w", encoding='utf-8') as f:
    json.dump(mtime, f, indent=2)

if __name__ == '__main__':
  main()

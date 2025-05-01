#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

SRC_DIR = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(__file__)))))

args = [sys.executable, f"{SRC_DIR}/build/android/fast_local_dev_server.py"]
sys.exit(subprocess.call(args))

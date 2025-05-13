#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import subprocess
import sys

SRC_DIR = os.path.abspath(os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(__file__)))))

sys.path.insert(0, SRC_DIR)

from build.android.fast_local_dev_server import main, _exception_hook

if __name__ == '__main__':
  sys.excepthook = _exception_hook
  sys.exit(main())

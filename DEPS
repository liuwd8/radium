gclient_gn_args_from = 'src'


vars = {
  'chromium_revision': '1816bb54c1da55d17ed43b2d8f664a732121de01',

  'chromium_git': 'https://chromium.googlesource.com',

  # To be able to build clean Chromium from sources.
  'apply_patches': True,

  # To allow running hooks without parsing the DEPS tree
  'process_deps': True,

  # To allow in-house builds to checkout those manually.
  'checkout_chromium': True,
}

deps = {
  'src': {
    'url': (Var("chromium_git")) + '/chromium/src.git@' + (Var("chromium_revision")),
    'condition': 'checkout_chromium'
  }
}

pre_deps_hooks = [
]

hooks = [
  {
    'name': 'patch_chromium',
    'condition': 'checkout_chromium and apply_patches',
    'pattern': 'src/radium',
    'action': [
      'python3',
      'src/radium/tools/scripts/apply_all_patches.py',
      '--revision',
      Var("chromium_revision"),
      'src/radium/patches',
    ],
  }
]

recursedeps = [
  'src',
]
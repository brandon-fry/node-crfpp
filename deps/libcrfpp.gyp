{
  'target_defaults': {
    'default_configuration': 'Debug',
    'configurations': {
      'Debug': {
        'defines': [ 'DEBUG', '_DEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 1, # static debug
          },
        },
      },
      'Release': {
        'defines': [ 'NDEBUG' ],
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 0, # static release
          },
        },
      }
    },
    'msvs_settings': {
      'VCLinkerTool': {
        'GenerateDebugInformation': 'true',
      },
    },
    'include_dirs': [
      '../libdvbtee/libdvbpsi/src',
      '../libdvbtee/libdvbpsi/src/tables',
      '../libdvbtee/libdvbpsi/src/descriptors',
      '../libdvbtee/libdvbpsi'
    ],
    'defines': [
      'PIC',
      'HAVE_CONFIG_H'
    ],
  },

  'targets': [
    # crfpp
    {
      'target_name': 'crfpp',
      'product_prefix': 'lib',
      'type': 'static_library',
      'sources': [
        'crfpp/libcrfpp.cpp',
        'crfpp/lbfgs.cpp',
        'crfpp/param.cpp',
        'crfpp/encoder.cpp',
        'crfpp/feature.cpp',
        'crfpp/feature_cache.cpp',
        'crfpp/feature_index.cpp',
        'crfpp/node.cpp',
        'crfpp/path.cpp',
        'crfpp/tagger.cpp',
      ],
      'conditions': [
        ['OS=="mac"',
          {
            'xcode_settings': {
              'WARNING_CFLAGS': [
                '-Wno-deprecated-declarations'
              ]
            }
          }
        ]
      ],
      'cflags!': ['-Wdeprecated-declarations','-Wimplicit-function-declaration'],
      'cflags+': ['-Wno-deprecated-declarations'],
    },
  ]
}

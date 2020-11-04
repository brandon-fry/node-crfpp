{
  'target_defaults': {
    'default_configuration': 'Release',
    'configurations': {
      'Debug': {
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 1,
          },
          'VCLinkerTool': {
            'GenerateDebugInformation': 'true',
          },
        },
      },
      'Release': {
        'msvs_settings': {
          'VCCLCompilerTool': {
            'RuntimeLibrary': 0, # static release
          },
        },
      }
    },
    'defines': [
      'HAVE_CONFIG_H'
    ],
  },

  'targets': [
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

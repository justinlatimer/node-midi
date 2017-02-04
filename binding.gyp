{
  'targets': [
    {
      'target_name': 'midi',
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        'src'
      ],
      'sources': [
        'src/node-midi.cpp'
      ],
      'conditions': [
        ['OS=="linux"',
          {
            'cflags_cc!': [
              '-fno-exceptions'
            ],
            'defines': [
              '__LINUX_ALSA__',
              '__UNIX_JACK__'
            ],
            'link_settings': {
              'libraries': [
                '-lasound',
                '-lpthread',
                '-ljack'
              ]
            }
          }
        ],
        ['OS=="mac"',
          {
            'defines': [
              '__MACOSX_CORE__'
            ],
            'xcode_settings': {
              'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
            },
            'link_settings': {
              'libraries': [
                'CoreMIDI.framework',
                'CoreAudio.framework',
                'CoreFoundation.framework',
              ],
            }
          }
        ],
        ['OS=="win"',
          {
            'configurations': {
              'Release': {
                'msvs_settings': {
                  'VCCLCompilerTool': {
                    'ExceptionHandling': 1
                  }
                }
              }
            },
            'defines': [
              '__WINDOWS_MM__'
            ],
            'link_settings': {
              'libraries': [
                '-lwinmm.lib'
              ],
            }
          }
        ]
      ]
    }
  ]
}

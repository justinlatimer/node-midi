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
              '__LINUX_ALSA__'
            ],
            'link_settings': {
              'libraries': [
                '-lasound',
                '-lpthread',
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
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}

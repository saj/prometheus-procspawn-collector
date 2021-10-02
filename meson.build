project('prometheus-procspawn-collector', 'c',
    version: '0.1.0',
    default_options: [
        'buildtype=release',
        'c_std=c17',
    ])

lib_sds = static_library('sds', 'thirdparty/sds/sds.c')

test_label = executable('test_label',
    ['src/label.c', 'src/label_test.c'],
    link_with: lib_sds)
test('label', test_label)

test_path = executable('test_path',
    ['src/path.c', 'src/path_test.c'],
    link_with: lib_sds)
test('path', test_path)

executable('prometheus-procspawn-collector',
    [
        'src/error.c',
        'src/label.c',
        'src/main.c',
        'src/path.c',
        'src/signal.c',
    ],
    link_with: lib_sds,
    install: true)

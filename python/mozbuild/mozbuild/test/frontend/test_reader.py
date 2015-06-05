# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from __future__ import unicode_literals

import os
import sys
import unittest

from mozunit import main

from mozbuild.frontend.reader import BuildReaderError
from mozbuild.frontend.reader import BuildReader

from mozbuild.test.common import MockConfig

import mozpack.path as mozpath


if sys.version_info.major == 2:
    text_type = 'unicode'
else:
    text_type = 'str'

data_path = mozpath.abspath(mozpath.dirname(__file__))
data_path = mozpath.join(data_path, 'data')


class TestBuildReader(unittest.TestCase):
    def config(self, name, **kwargs):
        path = mozpath.join(data_path, name)

        return MockConfig(path, **kwargs)

    def reader(self, name, enable_tests=False, **kwargs):
        extra = {}
        if enable_tests:
            extra['ENABLE_TESTS'] = '1'
        config = self.config(name, extra_substs=extra)

        return BuildReader(config, **kwargs)

    def file_path(self, name, *args):
        return mozpath.join(data_path, name, *args)

    def test_dirs_traversal_simple(self):
        reader = self.reader('traversal-simple')

        contexts = list(reader.read_topsrcdir())

        self.assertEqual(len(contexts), 4)

    def test_dirs_traversal_no_descend(self):
        reader = self.reader('traversal-simple')

        path = mozpath.join(reader.config.topsrcdir, 'moz.build')
        self.assertTrue(os.path.exists(path))

        contexts = list(reader.read_mozbuild(path, reader.config,
            descend=False))

        self.assertEqual(len(contexts), 1)

    def test_dirs_traversal_all_variables(self):
        reader = self.reader('traversal-all-vars', enable_tests=True)

        contexts = list(reader.read_topsrcdir())
        self.assertEqual(len(contexts), 3)

    def test_relative_dirs(self):
        # Ensure relative directories are traversed.
        reader = self.reader('traversal-relative-dirs')

        contexts = list(reader.read_topsrcdir())
        self.assertEqual(len(contexts), 3)

    def test_repeated_dirs_ignored(self):
        # Ensure repeated directories are ignored.
        reader = self.reader('traversal-repeated-dirs')

        contexts = list(reader.read_topsrcdir())
        self.assertEqual(len(contexts), 3)

    def test_outside_topsrcdir(self):
        # References to directories outside the topsrcdir should fail.
        reader = self.reader('traversal-outside-topsrcdir')

        with self.assertRaises(Exception):
            list(reader.read_topsrcdir())

    def test_error_basic(self):
        reader = self.reader('reader-error-basic')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertEqual(e.actual_file, self.file_path('reader-error-basic',
            'moz.build'))

        self.assertIn('The error occurred while processing the', str(e))

    def test_error_included_from(self):
        reader = self.reader('reader-error-included-from')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertEqual(e.actual_file,
            self.file_path('reader-error-included-from', 'child.build'))
        self.assertEqual(e.main_file,
            self.file_path('reader-error-included-from', 'moz.build'))

        self.assertIn('This file was included as part of processing', str(e))

    def test_error_syntax_error(self):
        reader = self.reader('reader-error-syntax')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('Python syntax error on line 5', str(e))
        self.assertIn('    foo =', str(e))
        self.assertIn('         ^', str(e))

    def test_error_read_unknown_global(self):
        reader = self.reader('reader-error-read-unknown-global')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('The error was triggered on line 5', str(e))
        self.assertIn('The underlying problem is an attempt to read', str(e))
        self.assertIn('    FOO', str(e))

    def test_error_write_unknown_global(self):
        reader = self.reader('reader-error-write-unknown-global')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('The error was triggered on line 7', str(e))
        self.assertIn('The underlying problem is an attempt to write', str(e))
        self.assertIn('    FOO', str(e))

    def test_error_write_bad_value(self):
        reader = self.reader('reader-error-write-bad-value')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('The error was triggered on line 5', str(e))
        self.assertIn('is an attempt to write an illegal value to a special',
            str(e))

        self.assertIn('variable whose value was rejected is:\n\n    DIRS',
            str(e))

        self.assertIn('written to it was of the following type:\n\n    %s' % text_type,
            str(e))

        self.assertIn('expects the following type(s):\n\n    list', str(e))

    def test_error_illegal_path(self):
        reader = self.reader('reader-error-outside-topsrcdir')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('The underlying problem is an illegal file access',
            str(e))

    def test_error_missing_include_path(self):
        reader = self.reader('reader-error-missing-include')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('we referenced a path that does not exist', str(e))

    def test_error_script_error(self):
        reader = self.reader('reader-error-script-error')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('The error appears to be the fault of the script',
            str(e))
        self.assertIn('    ["TypeError: unsupported operand', str(e))

    def test_error_bad_dir(self):
        reader = self.reader('reader-error-bad-dir')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('we referenced a path that does not exist', str(e))

    def test_error_repeated_dir(self):
        reader = self.reader('reader-error-repeated-dir')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('Directory (foo) registered multiple times in DIRS',
            str(e))

    def test_error_error_func(self):
        reader = self.reader('reader-error-error-func')

        with self.assertRaises(BuildReaderError) as bre:
            list(reader.read_topsrcdir())

        e = bre.exception
        self.assertIn('A moz.build file called the error() function.', str(e))
        self.assertIn('    Some error.', str(e))

    def test_inheriting_variables(self):
        reader = self.reader('inheriting-variables')

        contexts = list(reader.read_topsrcdir())

        self.assertEqual(len(contexts), 4)
        self.assertEqual([context.relsrcdir for context in contexts],
            ['', 'foo', 'foo/baz', 'bar'])
        self.assertEqual([context['XPIDL_MODULE'] for context in contexts],
            ['foobar', 'foobar', 'baz', 'foobar'])

    def test_process_eval_callback(self):
        def strip_dirs(context):
            context['DIRS'][:] = []
            count[0] += 1

        reader = self.reader('traversal-simple',
            sandbox_post_eval_cb=strip_dirs)

        count = [0]

        contexts = list(reader.read_topsrcdir())

        self.assertEqual(len(contexts), 1)
        self.assertEqual(len(count), 1)


if __name__ == '__main__':
    main()

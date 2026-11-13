import unittest
from check_formula_parameter_layouts import compare_buffers


class ParameterLayoutTests(unittest.TestCase):
    def test_compares_relative_pvar_window_including_both_edges(self):
        pascal = bytes(512)
        cpp = bytearray(1024)
        cpp[0] = cpp[1023] = 1  # Outside the Pascal allocation window.
        self.assertEqual(compare_buffers(pascal, cpp)['status'], 'pass')
        cpp[256] = cpp[512 + 24] = cpp[767] = 1
        self.assertEqual(compare_buffers(pascal, cpp)['pvar_relative_offsets'], [-256, 24, 255])

    def test_truncated_dumps_cannot_pass(self):
        for p, c in [(bytes(511), bytes(1024)), (bytes(512), bytes(1023))]:
            with self.assertRaises(ValueError):
                compare_buffers(p, c)


if __name__ == '__main__':
    unittest.main()

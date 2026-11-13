"""Pinned real-map inputs and raw device evidence for formula observations.

This validates loading and execution, not the independent sampling arithmetic.
Only explicit PNG fixtures are supported; no implicit search or missing-map
substitution can supply a successful resource observation.
"""
import hashlib
from pathlib import Path
import re
import struct
import zlib

from qualify_native_metal_all import artifact, require

SCHEMA = 'mb3d.native-formula-map-input.v1'
FIELDS = {'schema', 'root', 'map_ids', 'files', 'descriptor_words',
          'pixel_count', 'pixel_words_sha256', 'controls'}
CONTROLS = ('reserved', 'channels', 'offset', 'flags', 'extent',
            'element_count', 'missing', 'empty_reserved', 'empty_channels',
            'empty_offset', 'nonfinite_pixel')


def uint_words(value, count):
    require(isinstance(value, list) and len(value) == count and
            all(type(x) is int and 0 <= x <= 0xffffffff for x in value),
            'Malformed resource uint32 words')
    return value


def pixel_hash(values):
    uint_words(values, len(values))
    return hashlib.sha256(struct.pack('<' + str(len(values)) + 'I', *values)).hexdigest()


def validate_png(path):
    """Canonical RGB8/16 fixture, verified before any loader search occurs."""
    data = Path(path).read_bytes()
    require(data[:8] == b'\x89PNG\r\n\x1a\n', 'Resource is not a PNG fixture')
    chunks, position = [], 8
    while position < len(data):
        require(position + 12 <= len(data), 'Truncated PNG chunk')
        size = struct.unpack_from('>I', data, position)[0]
        end = position + 12 + size
        require(end <= len(data), 'Truncated PNG payload')
        tag = data[position+4:position+8]
        payload = data[position+8:end-4]
        checksum = struct.unpack_from('>I', data, end-4)[0]
        require(zlib.crc32(tag + payload) == checksum, 'PNG CRC mismatch')
        chunks.append((tag, payload)); position = end
    require([tag for tag, _ in chunks] == [b'IHDR', b'IDAT', b'IEND'] and
            len(chunks[0][1]) == 13 and not chunks[2][1], 'Noncanonical PNG fixture chunks')
    width, height, depth, colour, compression, filtering, interlace = struct.unpack('>IIBBBBB', chunks[0][1])
    require(1 <= width <= 16384 and 1 <= height <= 16384 and depth in (8, 16) and
            (colour, compression, filtering, interlace) == (2, 0, 0, 0),
            'Expected noninterlaced RGB8/16 PNG fixture')
    stride = 1 + width * 3 * (depth // 8)
    expected = stride * height
    require(expected <= 1 << 26, 'PNG fixture exceeds bounded decode size')
    decoder = zlib.decompressobj()
    try:
        pixels = decoder.decompress(chunks[1][1], expected + 1)
    except zlib.error as error:
        raise ValueError('Invalid PNG compression') from error
    require(len(pixels) == expected and decoder.eof and not decoder.unused_data and
            not decoder.unconsumed_tail and all(pixels[i] == 0 for i in range(0, expected, stride)),
            'Expected complete filter-zero PNG fixture pixels')
    return width, height, depth


def validate(resources, root):
    require(isinstance(resources, dict) and set(resources) == FIELDS and
            resources['schema'] == SCHEMA and resources['controls'] == 'active',
            'Expected explicit active resource input')
    root = Path(root).resolve()
    rel = resources['root']
    require(isinstance(rel, str) and rel and not Path(rel).is_absolute() and
            '..' not in Path(rel).parts, 'Invalid resource root')
    runtime = (root / rel).resolve()
    require(runtime.is_relative_to(root) and runtime != root and runtime.is_dir(),
            'Escaping or absent resource root')
    ids = resources['map_ids']
    require(isinstance(ids, list) and 1 <= len(ids) <= 4 and
            all(type(i) is int and 0 <= i <= 0x7fffffff for i in ids) and
            len(set(ids)) == len(ids) and sum(i != 0 for i in ids) <= 3,
            'Invalid explicit map IDs')
    files = resources['files']
    require(isinstance(files, list) and len(files) == sum(i != 0 for i in ids),
            'Incomplete resource file manifest')
    paths = []
    for record in files:
        require(isinstance(record, dict) and set(record) == {'path', 'sha256'},
                'Malformed resource file record')
        path = artifact(root, record).resolve()
        require(path.is_relative_to(runtime), 'Resource file outside explicit root')
        paths.append(path)
    require(len(set(paths)) == len(paths) and set(paths) ==
            {(runtime / 'M3Maps' / f'{i}.png').resolve() for i in ids if i},
            'Explicit map IDs and captured PNG files differ')
    entries = list(runtime.rglob('*'))
    require(all(not p.is_symlink() for p in entries) and
            {p.resolve() for p in entries if p.is_file()} == set(paths) and
            all(p.is_file() or p == runtime / 'M3Maps' for p in entries),
            'Unpinned competing resource or search-root entry')
    png_shapes = {int(path.stem): validate_png(path) for path in paths}
    descriptors = resources['descriptor_words']
    require(isinstance(descriptors, list) and len(descriptors) == len(ids),
            'Incomplete expected resource descriptors')
    offset = 0
    for identity, row in zip(ids, descriptors):
        uint_words(row, 8)
        number, width, height, channels, start, count, flags, reserved = row
        require(number == identity and channels == 3 and reserved == 0,
                'Invalid expected resource descriptor')
        if not identity:
            require(row == [0, 0, 0, 3, 0, 0, 2, 0], 'Malformed empty-map descriptor')
        else:
            require(1 <= width <= 16384 and 1 <= height <= 16384 and flags in (0, 1) and
                    start == offset and count == (width + 1) * (height + 1) * 3,
                    'Invalid expected resource extent/layout')
            require(png_shapes[identity] == (width, height, 16 if flags else 8),
                    'PNG dimensions/depth and expected upload disagree')
            offset += count
    require(type(resources['pixel_count']) is int and resources['pixel_count'] == offset and
            offset <= 1 << 26 and isinstance(resources['pixel_words_sha256'], str) and
            re.fullmatch(r'[0-9a-f]{64}', resources['pixel_words_sha256']) is not None,
            'Invalid expected resource pixel payload')
    return runtime, paths


def arguments(resources, root):
    runtime, _ = validate(resources, root)
    result = ['--resource-root', str(runtime), '--map-controls', 'active']
    for identity in resources['map_ids']:
        result += ['--map-id', str(identity)]
    return result


def validate_report(report, resources, root):
    runtime, _ = validate(resources, root)
    require(uint_words(report.get('map_abi'), 4) == [1, 32, 3, 1], 'Wrong actual resource ABI')
    actual = report.get('resources')
    require(isinstance(actual, dict) and
            actual.get('schema') == 'mb3d.native-formula-resources.v1' and
            actual.get('resource_root') == str(runtime) and
            actual.get('loader') == 'LoadLightMapNr via fresh Native RenderContext' and
            actual.get('requested_map_ids') == resources['map_ids'] and
            actual.get('descriptor_words') == resources['descriptor_words'] and
            type(actual.get('map_count')) is int and actual['map_count'] == len(resources['map_ids']) and
            type(actual.get('pixel_count')) is int and actual['pixel_count'] == resources['pixel_count'],
            'Loaded resources differ from independent resource input')
    require(all(type(i) is int for i in actual['requested_map_ids']), 'Malformed loaded map IDs')
    for descriptor in actual['descriptor_words']:
        uint_words(descriptor, 8)
    pixels = uint_words(actual.get('pixel_words'), resources['pixel_count'])
    require(pixel_hash(pixels) == resources['pixel_words_sha256'],
            'Loaded resource pixels differ from independent input')
    require(all(actual.get(key) is True for key in
                ('input_immutable', 'allocation_canaries', 'program_immutable', 'exact_control_repeats')) and
            actual.get('control_mode') == 'active', 'Inactive resource integrity controls')
    names = CONTROLS if pixels else CONTROLS[:-1]
    controls = actual.get('controls')
    require(isinstance(controls, list) and len(controls) == len(names),
            'Incomplete active resource controls')
    for control, name in zip(controls, names):
        fault = 7 if name == 'missing' else 4 if name == 'nonfinite_pixel' else 9
        require(isinstance(control, dict) and control.get('name') == name and
                type(control.get('expected_fault')) is int and control['expected_fault'] == fault and
                type(control.get('repeats')) is int and control['repeats'] == 3 and
                isinstance(control.get('rows'), list) and len(control['rows']) == 1,
                'Wrong active resource control')
        row = control['rows'][0]
        require(isinstance(row, dict) and type(row.get('index')) is int and row['index'] == 0,
                'Wrong resource control row')
        result = uint_words(row.get('result_words'), 59)
        require(type(row.get('fault')) is int and row['fault'] == result[47] == fault and
                type(row.get('work')) is int and row['work'] == result[51] > 0 and
                type(row.get('completed')) is int and row['completed'] == result[56] == 0,
                'Resource failure control produced a successful or different result')
    return actual


def validate_arguments(event, resources, root):
    expected = arguments(resources, root)
    argv = event['argv']
    selected = []
    for index, value in enumerate(argv):
        if value in ('--resource-root', '--map-controls', '--map-id'):
            require(index + 1 < len(argv), 'Missing resource argument value')
            selected += [value, argv[index + 1]]
    require(selected == expected, 'Actual resource arguments differ from input')
    require(all(record in event['inputs'] for record in resources['files']),
            'Uncaptured actual resource files')

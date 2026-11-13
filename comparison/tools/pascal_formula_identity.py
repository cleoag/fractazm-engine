"""Reject reference renders whose loader silently disabled an external formula."""
import ntpath
import re
import hashlib


# These are file-load migrations, not interchangeable formula implementations.
# FileHandling.ParseExternFormulas replaces each legacy name and sets its first
# option to zero when the replacement asset loads successfully (lines1874..1888).
# Record the migration explicitly; never relabel the image as a direct-kernel
# witness for the saved legacy name or accept a different axis/reverse alias.
LEGACY_FLIP_MIGRATIONS = {
    '_flipxy': '_FlipXYc',
    '_flipxz': '_FlipXZc',
    '_flipyz': '_FlipYZc',
}


def loaded_formulas(log, scene):
    key = ntpath.normcase(ntpath.normpath(str(scene)))
    selected = False
    found = {}
    for line in log.splitlines():
        marker = '[MB3D] Loading parameters: '
        if marker in line:
            selected = ntpath.normcase(ntpath.normpath(line.split(marker, 1)[1].strip())) == key
            if selected:
                found = {}  # Only the last load of this scene can describe its output.
        elif selected:
            match = re.search(r'Formula\[(\d+)\]: iFnr=(-?\d+) iItCount=(-?\d+) name=(.*)$', line)
            if match:
                slot, number, count, name = match.groups()
                found[int(slot)] = dict(number=int(number), iterations=int(count), name=name.strip())
    return found


def validate_formula_load(log, scene, slots):
    # HeaderTrafos.Ini1CFfromHAddon normalizes saved external IDs (e.g. 25)
    # to 20 after successful loading. CustomFormulas treats every ID >=20
    # as external; comparing only saved ID 20 would silently skip real scenes.
    expected = [s for s in slots if s['number'] >= 20 and s['iterations'] > 0]
    found = loaded_formulas(log, scene)
    migrations = []
    for slot in expected:
        actual = found.get(slot['slot'])
        expected_name = slot['name']
        migrated = LEGACY_FLIP_MIGRATIONS.get(expected_name.casefold())
        if actual and migrated and actual['name'].casefold() == migrated.casefold():
            expected_name = migrated
            migrations.append(dict(slot=slot['slot'], saved_name=slot['name'],
                loaded_name=migrated, option_0=0,
                source='mb3d/FileHandling.pas:ParseExternFormulas',
                scope='file-load migration; not direct legacy-kernel identity'))
        if (not actual or actual['number'] != 20 or actual['iterations'] <= 0
                or actual['name'].casefold() != expected_name.casefold()):
            raise ValueError('Pascal reference formula load mismatch: slot '
                             + str(slot['slot']) + ' expected ' + slot['name']
                             + ', loaded ' + str(actual))
    return {'verified': True, 'external_slots': [s['slot'] for s in expected],
            'loaded_slots': found, 'file_load_migrations': migrations}


def validate_reference_formula_load(directory, case, scene, slots):
    error = case.get('error', '')
    if error.startswith('Pascal reference formula load mismatch:'):
        raise ValueError(error)
    if slots is None:
        return None  # Older manifests have no slot inventory to verify.
    name = case.get('pascal_log')
    if name:
        raw = (directory / name).read_bytes()
        if hashlib.sha256(raw).hexdigest() != case.get('pascal_log_sha256'):
            raise ValueError('Pascal reference load log hash changed')
    elif 'batch' in case and 'retry_ms' not in case:
        path = directory / f"batch-{case['batch']:05d}.log"
        if not path.exists():
            return None
        raw = path.read_bytes()
    else:
        # Historical retry logs were not retained. Do not invent evidence.
        return None
    return validate_formula_load(raw.decode(errors='replace'), scene, slots)

"""Audited scalar MAP labels; this does not change Pascal map-loading metadata.

Calc.pas IniCalcMaps matches any integer label containing MAP. The four Tglad
CODE blocks use Log2MapPower as an integer exponent and contain no callback or
indirect jump. Bind the exception to exact audited bytes: both the repository's
LF blobs and the corresponding CRLF checkout bytes. No runtime normalization
is performed; any mutation or mixed line endings produces an unapproved hash.
"""
SCALAR_MAP_ASSETS = {
    'TgladTetra': 'f7799919a51b20a7ed091fb4efc260c85edc20f431f20b6e8a8bb4ce68e57872',
    'TgladTetra2': '0032746209b50cae45dad8db001fe4adfd2b6a0206a46a2b226015124f8aa8f8',
    'TgladDihed': 'e07e79e187635f62dd67a348f74749bda9f5006587197a51d2b2f8910efe7b90',
    'TgladDihed2': '95c4a94aa281b8213dfef3101e2e1a97b7989c2338c2d72cd54d1d1866085441',
}


# Preserve SCALAR_MAP_ASSETS' existing name -> CRLF digest interface. These
# additional literals were audited against the immutable Git LF blobs; replacing
# each LF with CRLF reproduces the corresponding existing digest exactly.
_SCALAR_MAP_ASSETS_LF = {
    'TgladTetra': '8ea4261e94c77ba5ad796d47e0cab56e2bf5d29e155a6070ed244ddf9b411c1a',
    'TgladTetra2': '1cb30986ce5ff6d96a542e64405abeb157937f8398ef0f2fab365310a0d32ce6',
    'TgladDihed': '131df2693eb3b693f9d7151929d44b7ea11f8df95d399e10c89b8c4ac21ba883',
    'TgladDihed2': '036708ba5d8abcf1d8fa4eaa98332bc8136ddce1caf7276f0630945a77c20f54',
}


def scalar_map_context(name, asset_sha256):
    return name in SCALAR_MAP_ASSETS and asset_sha256 in (
        SCALAR_MAP_ASSETS[name], _SCALAR_MAP_ASSETS_LF[name])

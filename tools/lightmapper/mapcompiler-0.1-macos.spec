# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['map_compiler.py'],
    pathex=[],
    binaries=[('/opt/homebrew/Cellar/glfw/3.4/lib/libglfw.3.4.dylib', '.'), ('/Users/fabiandepaoli/Library/Mobile Documents/com~apple~CloudDocs/SharedData/HM/GamesEngineering/Bayernstein/tools/lightmapper/souper/bin/souper-macos', '.')],
    datas=[('/Users/fabiandepaoli/Library/Mobile Documents/com~apple~CloudDocs/SharedData/HM/GamesEngineering/Bayernstein/tools/lightmapper/shaders', 'shaders')],
    hiddenimports=['numpy', 'numpy.core._multiarray_umath', 'contextvars', '_contextvars'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['ipykernel'],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='mapcompiler-0.1-macos',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)

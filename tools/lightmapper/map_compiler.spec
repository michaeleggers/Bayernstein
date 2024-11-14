# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['map_compiler.py'],
    pathex=[],
    binaries=[('D:\\data\\Informatik\\GamesEngineering\\Bayernstein\\tools\\lightmapper\\glfw3.dll', '.'), ('D:\\data\\Informatik\\GamesEngineering\\Bayernstein\\tools\\lightmapper\\souper\\bin\\Debug\\souper.exe', 'souper/bin/Debug')],
    datas=[('D:\\data\\Informatik\\GamesEngineering\\Bayernstein\\tools\\lightmapper\\shaders', 'shaders')],
    hiddenimports=[],
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
    name='map_compiler',
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

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$vsCandidates = @(
    'C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat',
    'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat',
    'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat'
)
$vs = $vsCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $vs) { throw 'No se encontro Visual Studio Build Tools con C++.' }
New-Item -ItemType Directory -Force -Path (Join-Path $root 'build') | Out-Null
$sources = @(
    'src\f1_main_win32.c', 'src\f1_ui.c', 'src\gfx.c', 'src\f1_data.c',
    'src\f1_data_gen.c', 'src\f1_game.c', 'src\f1_race.c', 'src\f1_ai.c',
    'src\f1_car.c', 'src\f1_team.c', 'src\f1_save.c'
) -join ' '
$command = "call `"$vs`" x64 >nul && cd /d `"$root`" && cl /nologo /O2 /std:c11 /utf-8 /W3 /D_CRT_SECURE_NO_WARNINGS $sources /Fe:build\f1_pixel_manager.exe user32.lib gdi32.lib"
cmd.exe /c $command
if ($LASTEXITCODE -ne 0) { throw "La compilacion fallo con codigo $LASTEXITCODE" }
Write-Host 'Creado build\f1_pixel_manager.exe'

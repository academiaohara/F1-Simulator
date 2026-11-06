$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
& (Join-Path $root 'build_windows.ps1')
$release = Join-Path $root 'release\f1_pixel_manager'
if (Test-Path $release) { Remove-Item -Recurse -Force $release }
New-Item -ItemType Directory -Force -Path $release | Out-Null
Copy-Item (Join-Path $root 'build\f1_pixel_manager.exe') $release
Copy-Item (Join-Path $root 'README.md') $release
Copy-Item (Join-Path $root 'docs\GUIA.md') $release
Copy-Item (Join-Path $root 'data\LICENSE-f1-circuits.md') $release
Compress-Archive -Path "$release\*" -DestinationPath (Join-Path $root 'release\f1_pixel_manager.zip') -Force
Write-Host 'Creado release\f1_pixel_manager.zip'

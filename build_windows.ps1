# Envoltorio de build_windows.bat. Si PowerShell bloquea scripts, usa directamente
# build_windows.bat (doble clic) o:  powershell -ExecutionPolicy Bypass -File .\build_windows.ps1
$ErrorActionPreference = 'Stop'
& cmd.exe /c "`"$PSScriptRoot\build_windows.bat`""
if ($LASTEXITCODE -ne 0) { throw "La compilacion fallo con codigo $LASTEXITCODE" }

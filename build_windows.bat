@echo off
rem Compila build\f1_pixel_manager.exe. Doble clic o: build_windows.bat
rem Usa Visual Studio / Build Tools (cl) si existe; si no, MinGW-w64 (gcc).
setlocal EnableDelayedExpansion
cd /d "%~dp0"
if not exist build mkdir build

set "SOURCES=src\f1_main_win32.c src\f1_ui.c src\f1_art.c src\gfx.c src\f1_data.c src\f1_data_gen.c src\f1_game.c src\f1_race.c src\f1_ai.c src\f1_car.c src\f1_team.c src\f1_save.c"

rem --- 1) Visual Studio (cualquier edicion: Community, Pro, Enterprise o Build Tools)
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSPATH="
if exist "%VSWHERE%" (
  for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
)
if defined VSPATH (
  echo Usando Visual Studio en "!VSPATH!"
  call "!VSPATH!\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
  if errorlevel 1 goto :fail
  cl /nologo /O2 /std:c11 /utf-8 /W3 /D_CRT_SECURE_NO_WARNINGS /Fo:build\ %SOURCES% /Fe:build\f1_pixel_manager.exe /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib
  if errorlevel 1 goto :fail
  goto :ok
)

rem --- 2) MinGW-w64
where gcc >nul 2>nul
if not errorlevel 1 (
  echo Usando gcc ^(MinGW^)
  gcc -std=c11 -O2 -Wall -Wextra -mwindows -static %SOURCES% -o build\f1_pixel_manager.exe -luser32 -lgdi32
  if errorlevel 1 goto :fail
  goto :ok
)

echo.
echo No se encontro ningun compilador de C. Instala UNA de estas opciones:
echo   - Visual Studio Build Tools 2022 con "Desarrollo para el escritorio con C++"
echo   - MinGW-w64 (por ejemplo con "winget install BrechtSanders.WinLibs.POSIX.UCRT")
echo O descarga el .exe ya compilado desde la pestana Actions de GitHub (ver docs\GUIA.md).
exit /b 1

:ok
echo.
echo Creado build\f1_pixel_manager.exe
exit /b 0

:fail
echo.
echo La compilacion fallo.
exit /b 1

@echo off
setlocal

set "SRC_DIR=%~dp0"
:: Remove trailing backslash if present
if "%SRC_DIR:~-1%"=="\" set "SRC_DIR=%SRC_DIR:~0,-1%"
set "DIST_DIR=%SRC_DIR%\dist\windows"
set "BUILD_DIR=%SRC_DIR%\build-windows-local"

echo ==============================================
echo Building simple-ui for Windows (Local Native)
echo ==============================================

:: Add Qt and MinGW to PATH if found in standard C:\Qt
if exist "C:\Qt\6.11.2\mingw_64\bin" (
    set "PATH=C:\Qt\6.11.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;%PATH%"
)

:: Ensure dist directory exists
if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cd /d "%BUILD_DIR%"

cmake "%SRC_DIR%" -G "Ninja" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto error

cmake --build .
if errorlevel 1 goto error

copy /y "%BUILD_DIR%\simple-ui.exe" "%DIST_DIR%\simple-ui.exe"

echo.
echo Running windeployqt to bundle DLLs and QML dependencies...
windeployqt.exe --release --qmldir "%SRC_DIR%" "%DIST_DIR%\simple-ui.exe"

echo.
echo ==============================================
echo [SUCCESS] Windows package built at:
echo %DIST_DIR%
echo ==============================================
goto end

:error
echo [ERROR] Build failed!
exit /b 1

:end
endlocal

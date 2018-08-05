@echo off

REM What's your project name?
set EXENAME=config
set OUTDIR=%~dp0\..\..\..\..\Tools\ResourceCompiler\

set SOURCE_DIRS=%~dp0

set INCLUDE_PATHS=
set LIB_PATHS=/LIBPATH:"%~dp0\..\..\..\..\lib\bin"
set LIBRARIES=kernel32.lib ^
user32.lib ^
gdi32.lib ^
winspool.lib ^
comdlg32.lib ^
advapi32.lib ^
shell32.lib ^
ole32.lib ^
oleaut32.lib ^
uuid.lib ^
odbc32.lib ^
odbccp32.lib ^
winmm.lib ^
Shlwapi.lib ^
legacy_stdio_definitions.lib ^
aurora.lib

set COMPILER_ARGS=/MP /D NOMINMAX /D _CRT_SECURE_NO_WARNINGS /D "_UNICODE" /D "UNICODE" /GS /GL /analyze- /W3 /Gy /Zc:wchar_t /EHsc /MT /WX- /Zc:forScope /Gd /Oy- /Oi /Gm- /O2 /nologo /Zi

setlocal enableextensions enabledelayedexpansion

REM Parsing command line arguments
set ARG_COUNT=0
for %%x in (%*) do (
    set /A ARG_COUNT+=1
    set "ARGS[!ARG_COUNT!]=%%~x"
)

for /L %%i in (1,1,%ARG_COUNT%) do (
    if !ARGS[%%i]! EQU exe (
        set BUILD_TYPE=exe
    ) else if !ARGS[%%i]! EQU lib (
        set BUILD_TYPE=lib
    ) else if !ARGS[%%i]! EQU dll (
        set BUILD_TYPE=dll
    ) else if !ARGS[%%i]! EQU x86 (
        set "ARCHITECTURE="
        set "LINKARCH=X86"
    ) else if !ARGS[%%i]! EQU x64 (
        set "ARCHITECTURE=amd64"
        set "LINKARCH=X64"
    )
)

if not defined BUILD_TYPE (
    set BUILD_TYPE=exe
)

if not defined ARCHITECTURE (
    set "ARCHITECTURE="
    set "LINKARCH=X86"
)

set BUILDDIRNAME=build

if %BUILD_TYPE% EQU dll (
    set COMPILER_ARGS=%COMPILER_ARGS% /LD
    set BUILDDIRNAME=lib
) else if %BUILD_TYPE% EQU lib (
    set BUILDDIRNAME=lib
)

REM Searching for a toolset. Preferring the newest

if defined VS140COMNTOOLS (
    set VCVARSALLPATH= "%VS140COMNTOOLS%..\..\VC\vcvarsall"
) else if defined VS120COMNTOOLS (
    set VCVARSALLPATH= "%VS120COMNTOOLS%..\..\VC\vcvarsall"
) else if defined VS110COMNTOOLS (
    set VCVARSALLPATH= "%VS110COMNTOOLS%..\..\VC\vcvarsall"
) else if defined VS100COMNTOOLS (
    set VCVARSALLPATH= "%VS100COMNTOOLS%..\..\VC\vcvarsall"
) else if defined VS90COMNTOOLS (
    set VCVARSALLPATH= "%VS90COMNTOOLS%..\..\VC\vcvarsall"
)

if not defined VCVARSALLPATH (
    echo No build system been found, abort.
    exit /b 1
)

if not defined DevEnvDir (call %VCVARSALLPATH% %ARCHITECTURE%)

REM =============================================

REM Collect all source files
set SOURCES=
for %%x in (%SOURCE_DIRS%) do (
    for /F "delims=" %%A in ('dir /b /S "%%~x*.c" "%%~x*.cpp" "%%~x*.res" "%%~x*.def"') do (
        set SOURCES=!SOURCES! "%%A"
    )
)
REM for /F "delims=" %%A in ('dir /b *.c *.cpp *.res *.def') do set SOURCES=!SOURCES! "%~dp0\%%A"
REM for /F "delims=" %%A in ('dir /b /S "%~dp0\src\*.c" "%~dp0\src\*.cpp" "%~dp0\src\*.res" "%~dp0\src\*.def"') do set SOURCES=!SOURCES! "%%A"

if exist build.txt (
    set /p BUILDINDEX=<build.txt
    set BUILDDIR=%BUILDDIRNAME%\!BUILDINDEX!
) else (
    set BUILDDIR=%BUILDDIRNAME%
)


mkdir %OUTDIR%
mkdir obj
pushd obj

REM Compile
cl /c %INCLUDE_PATHS% ^
%COMPILER_ARGS% ^
%SOURCES%

REM Collect all obj files
set OBJS=
for /F "delims=" %%A in ('dir /b /S *.obj') do set OBJS=!OBJS! "%%A"

if %BUILD_TYPE% EQU exe (
    link /OUT:"%OUTDIR%%EXENAME%.exe" ^
    %OBJS% ^
    %LIBRARIES% ^
    /MACHINE:%LINKARCH% ^
    /OPT:REF ^
    /OPT:ICF ^
    /ERRORREPORT:PROMPT ^
    /NOLOGO ^
    %LIB_PATHS% ^
    /TLBID:1 ^
    /LTCG ^
    /DEBUG
) else if %BUILD_TYPE% EQU lib (
    lib /OUT:%OUTDIR%%EXENAME%.lib %OBJS% /LTCG
) else if %BUILD_TYPE% EQU dll (
    link /DLL /OUT:"%OUTDIR%%EXENAME%.dll" ^
    %OBJS% ^
    %LIBRARIES% ^
    /MACHINE:%LINKARCH% ^
    /OPT:REF ^
    /OPT:ICF ^
    /ERRORREPORT:PROMPT ^
    /NOLOGO ^
    %LIB_PATHS% ^
    /TLBID:1 ^
    /LTCG ^
    /DEBUG
)

if exist ..\build.txt (
    if %ERRORLEVEL% EQU 0 (
        popd
        set /a BUILDINDEX=!BUILDINDEX!+1
        >build.txt echo !BUILDINDEX!
    ) else (
        popd
    )
) else (
    popd
)
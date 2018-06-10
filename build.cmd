@echo off
setlocal EnableDelayedExpansion
cd %~dp0

set NUGET=tools\NuGet.exe
set NUGET_SRC=https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
set VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
set VSDEVCMD=

if exist "%VSWHERE%" (
  for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set VSDEVCMD=%%i\Common7\Tools\vsdevcmd.bat
  )
)

if not exist "%VSDEVCMD%" (
  echo Failed to find vsdevcmd.bat
  goto end
)

call "%VSDEVCMD%"


:build
set PLATFORMS=x86,x64
set CONFIGURATIONS=Debug,Release

for %%p in (%PLATFORMS%) do (
  for %%c in (%CONFIGURATIONS%) do (
    echo Building %%p^|%%c
    msbuild /p:Platform=%%p /p:Configuration=%%c /t:Rebuild /v:quiet /nologo test-cpp.sln
  )
)

:nuget
if not exist "%NUGET%" (
  echo Downloading nuget from %NUGET_SRC%
  mkdir %NUGET% 2>&1 >NUL
  rmdir %NUGET% 2>&1 >NUL
  powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%NUGET_SRC%', '%NUGET%')"
  echo.   Done.
)

"%NUGET%" pack test-cpp.nuspec -OutputDirectory ".bin"

if exist "%LOCAL_NUGET_REPO%" (
  echo Copying to local nuget repository %LOCAL_NUGET_REPO%
  xcopy ".bin\*.nupkg" "%LOCAL_NUGET_REPO%" /F /R /Y /D  2>&1 1>NUL
)

:end
@echo off

rem echo %1
rem echo %2
rem echo %3
rem echo %4
rem echo %5

%5 --bin --output %1\ndk_app.bin %3
%5 --text -acd --interleave=source --output %1\ndk_app.disasm %3

copy %4 %1\ndk_app.hex
IF %errorlevel% NEQ 0 (
  @ECHO Error occurred, errno is %errorlevel%, exit.
  exit /b %errorlevel%
)

python ..\..\..\..\scripts\signed_image.py %1\ndk_app.bin %1\ndk_app.signed.bin
IF %errorlevel% NEQ 0 (
  @ECHO Error occurred in signed_image.py, errno is %errorlevel%, exit.
  exit /b %errorlevel%
)

echo Post Build Complete!

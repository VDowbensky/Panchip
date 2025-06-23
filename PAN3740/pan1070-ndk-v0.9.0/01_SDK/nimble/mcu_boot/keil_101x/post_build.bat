@echo off

REM echo %1
REM echo %2
REM echo %3
REM echo %4
REM echo %5

%5 --bin --output "%1\%2.bin" %3
%5 --text -acd --interleave=source --output "%1\%2.disasm" %3

copy %4 "%1\%2.hex"
IF %errorlevel% NEQ 0 (
  @ECHO Error occurred, errno is %errorlevel%, exit.
  exit /b %errorlevel%
)

echo Post Build Complete!

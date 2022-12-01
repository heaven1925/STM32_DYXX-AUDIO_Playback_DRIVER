@echo off
REM ------------------------------------------------------------------------------------------------
REM **** DESCRIPTION   : Code Beautifier Script
REM **** CREATION DATE : 01/01/2016
REM **** VERSION       : 3
REM ------------------------------------------------------------------------------------------------

REM Add filenames to below variable to skip code beautification on specific source files
set EXCLUDED_FILES="stm32f7xx_hal_conf.h system_stm32f7xx.c system_stm32f7xx.h stm32f765xx.h stm32f7xx.h"

set FILETYPES=*.c;*.h;*.cpp
set THIS_DIR=%~dp0
set PROJECT_ROOT_DIR=%THIS_DIR%\..\..
set SOURCE_DIR=%PROJECT_ROOT_DIR%\Source
set TERM=msys
set TAG=[code_beautifier]

REM ===========================================================================

echo %TAG% Started at  %time%

REM create file list
cd "%SOURCE_DIR%"
dir /b /s %FILETYPES% | findstr /v /i %EXCLUDED_FILES% > "%THIS_DIR%\files.txt"
cd "%THIS_DIR%"

REM apply code beautification to each file
uncrustify.exe -q -c uncrustify.cfg -l C --no-backup -F files.txt

echo %TAG% Code beautified successfully

REM delete temporary file
del files.txt

REM echo %TAG% Code beautifier worked on %SOURCE_DIR%

echo %TAG% Finished at %time%

REM wait for keypress
REM pause

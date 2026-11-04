@echo off
rem Build everything with MSVC (Visual Studio 2022) + OpenCL (headers/lib in include/, lib/).
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if not exist bin mkdir bin

set CL=/nologo /O2 /std:c11 /I include
set LINK=/link lib\OpenCL.lib

cl %CL% src\md5_test.c src\md5_cpu.c /Fe:bin\md5_test.exe /Fo:bin\
cl %CL% src\det_bench.c src\determinant_cpu.c src\determinant_gpu.c src\matrix_util.c src\cl_util.c /Fe:bin\det_bench.exe /Fo:bin\ %LINK%
cl %CL% src\det_main.c src\determinant_cpu.c src\determinant_gpu.c src\matrix_util.c src\cl_util.c /Fe:bin\det_main.exe /Fo:bin\ %LINK%
cl %CL% src\hash_bench.c src\md5_cpu.c src\bruteforce_cpu.c src\bruteforce_gpu.c src\cl_util.c /Fe:bin\hash_bench.exe /Fo:bin\ %LINK%
cl %CL% src\hash_main.c src\md5_cpu.c src\bruteforce_cpu.c src\bruteforce_gpu.c src\cl_util.c /Fe:bin\hash_main.exe /Fo:bin\ %LINK%

echo.
echo Done:
echo   bin\md5_test.exe
echo   bin\det_bench.exe   (writes bin\det_results.csv)
echo   bin\det_main.exe [N]
echo   bin\hash_bench.exe  (writes bin\hash_results.csv)
echo   bin\hash_main.exe [password]
echo.
bin\md5_test.exe
if errorlevel 1 exit /b 1

@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" > nul

cl.exe /std:c++20 /EHsc /I . /I Source /I Source/Resources /I Source/External /Fe:test_runner.exe tests\test_model_security.cpp Source\Resources\XorDecryptor.cpp Source\Resources\AssetEncryptor.cpp Source\Resources\gltfLoader.cpp

if %ERRORLEVEL% EQU 0 (
    echo.
    echo =======================================
    echo Compilation successful! Running tests...
    echo =======================================
    test_runner.exe
    del test_runner.exe test_runner.obj test_model_security.obj XorDecryptor.obj AssetEncryptor.obj gltfLoader.obj
) else (
    echo Compilation failed!
    exit /b 1
)

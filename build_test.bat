@echo off
echo Building Simple Test
echo ====================

REM Check C++17 support
echo Checking C++17 support...
echo #include <iostream> > test_cpp17.cpp
echo int main() { auto [x, y] = std::make_pair(1, 2); return 0; } >> test_cpp17.cpp
g++ -std=c++17 -c test_cpp17.cpp -o test_cpp17.o 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Your compiler does not support C++17
    echo Please install Visual Studio 2017+ or MinGW-w64 with GCC 7+
    del test_cpp17.cpp test_cpp17.o 2>nul
    pause
    exit /b 1
)
del test_cpp17.cpp test_cpp17.o 2>nul
echo C++17 support confirmed!

REM Compile test
echo Compiling test...
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude test_simple.cpp -o test_simple.exe
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile test
    pause
    exit /b 1
)

echo Test compiled successfully!
echo Run with: test_simple.exe
pause

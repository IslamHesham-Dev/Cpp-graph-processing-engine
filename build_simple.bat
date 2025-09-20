@echo off
echo Building Graph Engine (Simple Build)
echo ====================================

REM Create output directory
if not exist "bin" mkdir bin
if not exist "obj" mkdir obj

REM Compiler settings
set CXX=g++
set CXXFLAGS=-std=c++17 -Wall -Wextra -O2 -Iinclude
set LDFLAGS=

REM Check if C++17 is supported
echo Checking C++17 support...
echo #include <iostream> > test_cpp17.cpp
echo int main() { auto [x, y] = std::make_pair(1, 2); return 0; } >> test_cpp17.cpp
%CXX% -std=c++17 -c test_cpp17.cpp -o test_cpp17.o 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Your compiler does not support C++17
    echo Please install Visual Studio 2017+ or MinGW-w64 with GCC 7+
    del test_cpp17.cpp test_cpp17.o 2>nul
    pause
    exit /b 1
)
del test_cpp17.cpp test_cpp17.o 2>nul
echo C++17 support confirmed!

REM Compile source files
echo Compiling source files...
%CXX% %CXXFLAGS% -c src/graph.cpp -o obj/graph.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile graph.cpp
    pause
    exit /b 1
)

%CXX% %CXXFLAGS% -c src/algorithms/traversal.cpp -o obj/traversal.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile traversal.cpp
    pause
    exit /b 1
)

%CXX% %CXXFLAGS% -c src/algorithms/shortest_path.cpp -o obj/shortest_path.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile shortest_path.cpp
    pause
    exit /b 1
)

%CXX% %CXXFLAGS% -c src/algorithms/mst.cpp -o obj/mst.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile mst.cpp
    pause
    exit /b 1
)

%CXX% %CXXFLAGS% -c src/algorithms/flow.cpp -o obj/flow.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile flow.cpp
    pause
    exit /b 1
)

%CXX% %CXXFLAGS% -c src/algorithms/advanced.cpp -o obj/advanced.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile advanced.cpp
    pause
    exit /b 1
)

REM Create static library
echo Creating static library...
ar rcs bin/libgraph_engine.a obj/*.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to create static library
    pause
    exit /b 1
)

REM Build demo application
echo Building demo application...
%CXX% %CXXFLAGS% examples/demo.cpp -Lbin -lgraph_engine -o bin/demo.exe
if %errorlevel% neq 0 (
    echo ERROR: Failed to build demo
    pause
    exit /b 1
)

REM Build example applications
echo Building example applications...
%CXX% %CXXFLAGS% examples/social_network.cpp -Lbin -lgraph_engine -o bin/social_network.exe
if %errorlevel% neq 0 (
    echo WARNING: Failed to build social_network example
)

%CXX% %CXXFLAGS% examples/route_planner.cpp -Lbin -lgraph_engine -o bin/route_planner.exe
if %errorlevel% neq 0 (
    echo WARNING: Failed to build route_planner example
)

%CXX% %CXXFLAGS% examples/dependency_resolver.cpp -Lbin -lgraph_engine -o bin/dependency_resolver.exe
if %errorlevel% neq 0 (
    echo WARNING: Failed to build dependency_resolver example
)

echo.
echo Build completed successfully!
echo =============================
echo.
echo Generated files:
echo - bin/libgraph_engine.a (static library)
echo - bin/demo.exe (demo application)
echo - bin/social_network.exe (social network example)
echo - bin/route_planner.exe (route planner example)
echo - bin/dependency_resolver.exe (dependency resolver example)
echo.
echo To run the demo:
echo   bin\demo.exe
echo.
pause

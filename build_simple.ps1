# PowerShell build script for Graph Engine
Write-Host "Building Graph Engine (Simple Build)" -ForegroundColor Green
Write-Host "====================================" -ForegroundColor Green

# Create output directories
if (!(Test-Path "bin")) { New-Item -ItemType Directory -Name "bin" }
if (!(Test-Path "obj")) { New-Item -ItemType Directory -Name "obj" }

# Compiler settings
$CXX = "g++"
$CXXFLAGS = @("-std=c++17", "-Wall", "-Wextra", "-O2", "-Iinclude")
$LDFLAGS = @()

# Check if C++17 is supported
Write-Host "Checking C++17 support..." -ForegroundColor Yellow
$testCode = @"
#include <iostream>
int main() { 
    auto [x, y] = std::make_pair(1, 2); 
    return 0; 
}
"@
$testCode | Out-File -FilePath "test_cpp17.cpp" -Encoding UTF8

try {
    & $CXX -std=c++17 -c test_cpp17.cpp -o test_cpp17.o 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "C++17 not supported"
    }
    Remove-Item "test_cpp17.cpp", "test_cpp17.o" -ErrorAction SilentlyContinue
    Write-Host "C++17 support confirmed!" -ForegroundColor Green
} catch {
    Write-Host "ERROR: Your compiler does not support C++17" -ForegroundColor Red
    Write-Host "Please install Visual Studio 2017+ or MinGW-w64 with GCC 7+" -ForegroundColor Red
    Remove-Item "test_cpp17.cpp", "test_cpp17.o" -ErrorAction SilentlyContinue
    Read-Host "Press Enter to exit"
    exit 1
}

# Function to compile source file
function Compile-Source {
    param($sourceFile, $objectFile)
    
    Write-Host "Compiling $sourceFile..." -ForegroundColor Yellow
    & $CXX $CXXFLAGS -c $sourceFile -o $objectFile
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Failed to compile $sourceFile" -ForegroundColor Red
        Read-Host "Press Enter to exit"
        exit 1
    }
}

# Compile source files
Compile-Source "src/graph.cpp" "obj/graph.o"
Compile-Source "src/algorithms/traversal.cpp" "obj/traversal.o"
Compile-Source "src/algorithms/shortest_path.cpp" "obj/shortest_path.o"
Compile-Source "src/algorithms/mst.cpp" "obj/mst.o"
Compile-Source "src/algorithms/flow.cpp" "obj/flow.o"
Compile-Source "src/algorithms/advanced.cpp" "obj/advanced.o"

# Create static library
Write-Host "Creating static library..." -ForegroundColor Yellow
$objectFiles = Get-ChildItem "obj/*.o" | ForEach-Object { $_.FullName }
& ar rcs "bin/libgraph_engine.a" $objectFiles

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to create static library" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Function to build executable
function Build-Executable {
    param($sourceFile, $outputFile)
    
    Write-Host "Building $outputFile..." -ForegroundColor Yellow
    & $CXX $CXXFLAGS $sourceFile -Lbin -lgraph_engine -o $outputFile
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "WARNING: Failed to build $outputFile" -ForegroundColor Yellow
    }
}

# Build applications
Build-Executable "examples/demo.cpp" "bin/demo.exe"
Build-Executable "examples/social_network.cpp" "bin/social_network.exe"
Build-Executable "examples/route_planner.cpp" "bin/route_planner.exe"
Build-Executable "examples/dependency_resolver.cpp" "bin/dependency_resolver.exe"

Write-Host ""
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "=============================" -ForegroundColor Green
Write-Host ""
Write-Host "Generated files:" -ForegroundColor Cyan
Write-Host "- bin/libgraph_engine.a (static library)" -ForegroundColor White
Write-Host "- bin/demo.exe (demo application)" -ForegroundColor White
Write-Host "- bin/social_network.exe (social network example)" -ForegroundColor White
Write-Host "- bin/route_planner.exe (route planner example)" -ForegroundColor White
Write-Host "- bin/dependency_resolver.exe (dependency resolver example)" -ForegroundColor White
Write-Host ""
Write-Host "To run the demo:" -ForegroundColor Cyan
Write-Host "  .\bin\demo.exe" -ForegroundColor White
Write-Host ""
Read-Host "Press Enter to continue"

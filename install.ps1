# Parameters
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$BUILD_TYPE = if ($env:BUILD_TYPE) { $env:BUILD_TYPE } else { "Release" }
$BUILD = if ($env:BUILD) { $env:BUILD } else { "build" }
$PARALLEL = if ($env:PARALLEL) { $env:PARALLEL } else { 8 }
$ConfirmSettings = $false
$PromptOnStaleCache = $true

function Write-Log {
    param([string]$Message)
    Write-Host "[install] $Message"
}

function Test-InteractiveHost {
    return $Host.Name -notmatch 'ServerRemoteHost|Visual Studio Code Host'
}

function Confirm-Choice {
    param(
        [string]$Prompt,
        [string]$Default = "Y"
    )

    while ($true) {
        $reply = Read-Host "$Prompt [$Default]"
        if ([string]::IsNullOrWhiteSpace($reply)) {
            $reply = $Default
        }

        switch ($reply.ToLowerInvariant()) {
            "y" { return $true }
            "yes" { return $true }
            "n" { return $false }
            "no" { return $false }
        }

        Write-Host "Please answer yes or no."
    }
}

function Get-CacheSourceDirectory {
    param([string]$CacheFile)

    if (-not (Test-Path $CacheFile)) {
        return $null
    }

    $line = Select-String -Path $CacheFile -Pattern '^CMAKE_HOME_DIRECTORY:INTERNAL=' | Select-Object -Last 1
    if ($null -eq $line) {
        return $null
    }

    return $line.Line -replace '^CMAKE_HOME_DIRECTORY:INTERNAL=', ''
}

function Ensure-FreshBuildDirectory {
    param([string]$BuildDir)

    $cacheFile = Join-Path $BuildDir "CMakeCache.txt"
    $cacheSource = Get-CacheSourceDirectory -CacheFile $cacheFile
    if ([string]::IsNullOrWhiteSpace($cacheSource) -or $cacheSource -eq $Root) {
        return
    }

    Write-Log "Detected a build cache from a different source directory."
    Write-Log "Current repo: $Root"
    Write-Log "Cached source: $cacheSource"

    if ($PromptOnStaleCache -and (Test-InteractiveHost)) {
        if (-not (Confirm-Choice "Reset the build directory and reconfigure?" "Y")) {
            throw "Installation cancelled."
        }
    }

    if (-not (Test-Path $BuildDir)) {
        return
    }

    Write-Log "Resetting stale CMake state in: $BuildDir"
    Write-Log "Keeping reusable downloaded dependencies under $BuildDir\_deps when present"

    $pathsToRemove = @(
        (Join-Path $BuildDir "CMakeCache.txt"),
        (Join-Path $BuildDir "CTestTestfile.cmake"),
        (Join-Path $BuildDir "Makefile"),
        (Join-Path $BuildDir "cmake_install.cmake"),
        (Join-Path $BuildDir "compile_commands.json"),
        (Join-Path $BuildDir "build.ninja"),
        (Join-Path $BuildDir ".ninja_deps"),
        (Join-Path $BuildDir ".ninja_log"),
        (Join-Path $BuildDir "CMakeFiles")
    )

    foreach ($path in $pathsToRemove) {
        if (Test-Path $path) {
            Remove-Item -Recurse -Force $path
        }
    }
}

for ($i = 0; $i -lt $args.Length; $i++) {
    switch ($args[$i]) {
        "--build-dir" {
            $i++
            $BUILD = $args[$i]
        }
        "--build-type" {
            $i++
            $BUILD_TYPE = $args[$i]
        }
        "--parallel" {
            $i++
            $PARALLEL = $args[$i]
        }
        "--interactive" {
            $ConfirmSettings = $true
        }
        "--no-prompt" {
            $ConfirmSettings = $false
            $PromptOnStaleCache = $false
        }
        "--help" {
            Write-Host "Usage: .\install.ps1 [--build-dir DIR] [--build-type TYPE] [--parallel N] [--interactive] [--no-prompt]"
            exit 0
        }
        default {
            throw "Unknown argument: $($args[$i])"
        }
    }
}

Set-Location $Root

if ($ConfirmSettings -and (Test-InteractiveHost)) {
    Write-Log "Planned install settings:"
    Write-Log "  Build directory: $BUILD"
    Write-Log "  Build type: $BUILD_TYPE"
    Write-Log "  Parallel jobs: $PARALLEL"

    if (-not (Confirm-Choice "Continue with these settings?" "Y")) {
        throw "Installation cancelled."
    }
}

if (-not ($PARALLEL -as [int]) -or [int]$PARALLEL -le 0) {
    throw "Parallel build jobs must be a positive integer."
}

Ensure-FreshBuildDirectory -BuildDir $BUILD

Write-Log "Configuring CMake in `"$BUILD`""
cmake -S $Root -B $BUILD -DCMAKE_BUILD_TYPE=$BUILD_TYPE

Write-Log "Building targets with $PARALLEL parallel jobs"
cmake --build $BUILD --parallel $PARALLEL --config $BUILD_TYPE

Write-Log "Copying executables to the repository root"
Copy-Item "$BUILD\cryfa.exe"  -Destination $Root -Force
Copy-Item "$BUILD\keygen.exe" -Destination $Root -Force

Write-Log "Install complete"

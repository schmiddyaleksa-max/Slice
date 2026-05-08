Param(
    [Parameter(Mandatory=$false)]
    [Switch] $clean,

    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help -eq $true) {
    Write-Output "`"Build`" - Compiles your mod into a `".so`" or a `".a`" library"
    Write-Output "`n-- Arguments --`n"
    Write-Output "-Clean `t`t Deletes the `"build`" folder, so that the entire library is rebuilt"
    exit
}

# Check that qpm is available
if (-not (Get-Command qpm -ErrorAction SilentlyContinue)) {
    Write-Error "qpm not found. Install it from: https://github.com/QuestPackageManager/QPM.CLI"
    exit 1
}

# Restore dependencies (generates extern/ shared/ qpm_defines.cmake extern.cmake)
Write-Output "Restoring QPM dependencies..."
& qpm restore

if ($LASTEXITCODE -ne 0) {
    Write-Error "qpm restore failed. Aborting build."
    exit 1
}

# if user specified clean, remove all build files
if ($clean.IsPresent) {
    if (Test-Path -Path "build") {
        Remove-Item build -Recurse -Force
    }
}

if (($clean.IsPresent) -or (-not (Test-Path -Path "build"))) {
    New-Item -Path build -ItemType Directory | Out-Null
}

& cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build

if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed."
    exit 1
}

& cmake --build ./build

if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
    exit 1
}

Write-Output "Build succeeded."
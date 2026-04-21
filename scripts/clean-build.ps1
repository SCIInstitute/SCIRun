# ============================================================
# Safe SCIRun build directory cleanup
# ============================================================

$ExpectedBuildDir = "C:\Users\Yong\Documents\SCIRun\build"

Write-Host "SCIRun safe cleanup script"
Write-Host "Target directory:"
Write-Host "  $ExpectedBuildDir"
Write-Host ""

# Resolve actual path
try {
    $Resolved = (Resolve-Path $ExpectedBuildDir).Path
} catch {
    Write-Error "ERROR: Build directory does not exist."
    exit 1
}

# Safety checks
if ($Resolved -ne $ExpectedBuildDir) {
    Write-Error "ERROR: Resolved path mismatch."
    Write-Error "Resolved: $Resolved"
    exit 1
}

if ($Resolved -match "^C:\\Users\\Yong\\?$") {
    Write-Error "ERROR: Refusing to operate on user directory."
    exit 1
}

if ($Resolved -ne "C:\Users\Yong\Documents\SCIRun\build") {
    Write-Error "ERROR: Refusing to clean unexpected path: $Resolved"
    exit 1
}

Write-Host "? Safety checks passed."
Write-Host ""

# Confirmation
$answer = Read-Host "Type 'DELETE BUILD DIR' to continue"
if ($answer -ne "DELETE BUILD DIR") {
    Write-Host "Aborted."
    exit 0
}

Write-Host ""
Write-Host "Cleaning build contents (directory preserved)..."
Write-Host ""

Get-ChildItem -Path $Resolved -Force |
    Remove-Item -Recurse -Force -ErrorAction Stop

Write-Host "? Build directory cleaned safely."

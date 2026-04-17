$BuildDir = "C:\Users\Yong\Documents\SCIRun\build"
$BoostDirs = @(
    "Boost_external",
    "boost",
    "_deps"
)

foreach ($name in $BoostDirs) {
    $path = Join-Path $BuildDir $name
    if (Test-Path $path) {
        Write-Host "Removing $path"
        Remove-Item -Recurse -Force $path
    }
}

Write-Host "? Boost-related directories removed."
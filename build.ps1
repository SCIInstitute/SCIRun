#Requires -Version 5.1

<#
.SYNOPSIS
    SCIRun Windows build script -- configure and build SCIRun using CMake and Visual Studio.

.DESCRIPTION
    Walks through the full SCIRun build pipeline on Windows, optionally downloading
    and installing missing prerequisites (Git, CMake, Visual Studio Build Tools, Qt).

    WHAT IT DOES
      1. Checks for Git, CMake 3.20+, Visual Studio C++ Build Tools, and Qt
      2. Offers to download and install anything missing (internet required)
      3. Configures the Superbuild with CMake
      4. Compiles SCIRun and all its dependencies
      5. Copies required DLLs to the output directory

    FIRST-TIME BUILD
      Expect 30-90 minutes: CMake downloads and compiles Boost, Python, Eigen, and
      other libraries from source before building SCIRun itself. Subsequent builds
      are much faster.

    DISK SPACE
      Allow at least 15 GB free in the build directory.

.PARAMETER Debug
    Build with debug symbols. Slower and larger, enables debugger use.

.PARAMETER Release
    Build an optimized release version (default).

.PARAMETER BuildVerbose
    Show full compiler command lines during the build.

.PARAMETER WithTetgen
    Include the Tetgen mesh generation library (GPL license).
    See https://tetgen.org for license details.

.PARAMETER Headless
    Build without the graphical user interface. Qt is not required.

.PARAMETER Documentation
    Build HTML/PDF documentation (requires a LaTeX installation).

.PARAMETER CMakePath
    Full path to cmake.exe if CMake is not on your PATH.
    Example: -CMakePath "C:\Program Files\CMake\bin\cmake.exe"

.PARAMETER CMakeArgs
    Extra CMake -D arguments to pass to the configure step.
    Example: -CMakeArgs "-DBUILD_TESTING=ON"

.PARAMETER QtPath
    Path to the Qt msvc directory containing 'lib' and 'bin' subdirectories.
    Example: -QtPath "C:\Qt\5.15.2\msvc2019_64"
    If omitted, common locations are searched automatically.

.PARAMETER Qt5
    Use Qt 5 (version 5.15.2). This is the default; the flag is a convenient
    shorthand when you want to be explicit or switch back from -Qt6.

.PARAMETER Qt6
    Use Qt 6 (version 6.3.1). Qt 6 has a more modern OpenGL stack and is the
    direction Qt development is heading, but is less battle-tested with SCIRun.
    Overrides -QtVersion if both are supplied.

.PARAMETER QtVersion
    Exact Qt version string to download if Qt is not already installed.
    Default: 5.15.2  (or 6.3.1 when -Qt6 is given)
    Supported values: 5.12.8  5.15.2  6.3.1
    Use -Qt5 / -Qt6 for the common case; -QtVersion for fine-grained control.

.PARAMETER BuildDir
    Directory for build files. Relative paths are resolved from the repo root.
    Default: <repo root>\bin

.PARAMETER Jobs
    Number of parallel compile jobs. Default: number of logical CPU cores.
    Lower this (e.g. -Jobs 4) if the build runs out of memory.

.PARAMETER InstallPrereqs
    Automatically install all missing prerequisites without prompting.
    Requires an internet connection and may trigger UAC elevation prompts.

.PARAMETER SkipVSInstall
    Do not offer to install Visual Studio Build Tools.

.PARAMETER SkipCMakeInstall
    Do not offer to install CMake.

.PARAMETER SkipQtInstall
    Do not offer to install Qt.

.EXAMPLE
    .\build.ps1
    Standard release build. Prompts before downloading anything.

.EXAMPLE
    .\build.ps1 -InstallPrereqs
    Installs everything needed automatically, then builds.

.EXAMPLE
    .\build.ps1 -Debug -Jobs 4
    Debug build limited to 4 parallel jobs (useful on low-memory machines).

.EXAMPLE
    .\build.ps1 -QtPath "C:\Qt\5.15.2\msvc2019_64" -BuildDir "C:\SCIRunBuild"
    Release build with explicit Qt path and a custom build directory.

.EXAMPLE
    .\build.ps1 -Qt6
    Release build using Qt 6.3.1 (downloaded automatically if not found).

.EXAMPLE
    .\build.ps1 -Headless
    Build without any GUI; no Qt required.
#>

param(
    [switch]$Debug,
    [switch]$Release,
    [switch]$BuildVerbose,
    [switch]$WithTetgen,
    [switch]$Headless,
    [switch]$Documentation,
    [string]$CMakePath    = "",
    [string]$CMakeArgs    = "",
    [string]$QtPath       = "",
    [switch]$Qt5,
    [switch]$Qt6,
    [string]$QtVersion    = "",
    [string]$BuildDir     = "",
    [int]$Jobs            = 0,
    [switch]$InstallPrereqs,
    [switch]$SkipVSInstall,
    [switch]$SkipCMakeInstall,
    [switch]$SkipQtInstall
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# --- Constants ---

$MIN_CMAKE_VERSION   = [Version]"3.20.0"
$DL_CMAKE_VERSION    = "3.31.4"
$DL_CMAKE_URL        = "https://github.com/Kitware/CMake/releases/download/v$DL_CMAKE_VERSION/cmake-$DL_CMAKE_VERSION-windows-x86_64.msi"
$DL_VS_BUILDTOOLS_URL = "https://aka.ms/vs/17/release/vs_buildtools.exe"
$DL_GIT_URL          = "https://github.com/git-for-windows/git/releases/download/v2.47.1.windows.1/Git-2.47.1-64-bit.exe"

# Script-level mutable state (avoids passing refs everywhere)
$script:cmakeBin    = "cmake"
$script:vsGenerator = ""
$script:qtFoundPath = ""

# --- Console helpers ---

function Write-Banner {
    $bar = "=" * 70
    Write-Host $bar -ForegroundColor Cyan
    Write-Host "  SCIRun Windows Build Script" -ForegroundColor Cyan
    Write-Host "  Scientific Computing and Imaging Institute, University of Utah" -ForegroundColor Cyan
    Write-Host $bar -ForegroundColor Cyan
    Write-Host ""
}

function Write-Step([string]$msg) {
    Write-Host ""
    Write-Host ">>> $msg" -ForegroundColor Cyan
}

function Write-OK([string]$msg)   { Write-Host "    [OK] $msg" -ForegroundColor Green  }
function Write-Info([string]$msg) { Write-Host "    [..] $msg" -ForegroundColor Gray   }
function Write-Warn([string]$msg) { Write-Host "    [!]  $msg" -ForegroundColor Yellow }

function Exit-WithError([string]$msg) {
    Write-Host ""
    Write-Host "    [ERROR] $msg" -ForegroundColor Red
    Write-Host ""
    Write-Host "    Build aborted. Fix the issue above and try again." -ForegroundColor Red
    Write-Host ""
    exit 1
}

# Returns $true if the user agrees (or -InstallPrereqs is set).
function Confirm-Install([string]$prompt) {
    if ($InstallPrereqs) { return $true }
    Write-Host ""
    $answer = Read-Host "    $prompt [Y/n]"
    return ($answer -eq "" -or $answer -match "^[Yy]")
}

function Test-IsAdmin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    $p  = [Security.Principal.WindowsPrincipal]$id
    return $p.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Enable-TLS12 {
    # Older .NET defaults may not use TLS 1.2; GitHub and cmake.org require it.
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
}

function Invoke-Download([string]$url, [string]$dest, [string]$label) {
    Enable-TLS12
    Write-Info "Downloading $label..."
    try {
        Invoke-WebRequest -Uri $url -OutFile $dest -UseBasicParsing
    } catch {
        Exit-WithError "Download failed for $label.`n    URL: $url`n    Error: $_"
    }
}

function Sync-EnvPath {
    $env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "Machine") + ";" +
                [System.Environment]::GetEnvironmentVariable("PATH", "User")
}

# --- Prerequisite detection ---

function Get-CMakeBin {
    if ($CMakePath -ne "" -and (Test-Path $CMakePath)) { return $CMakePath }
    $found = Get-Command cmake -ErrorAction SilentlyContinue
    if ($found) { return $found.Source }
    foreach ($loc in @(
        "$env:ProgramFiles\CMake\bin\cmake.exe",
        "${env:ProgramFiles(x86)}\CMake\bin\cmake.exe"
    )) {
        if (Test-Path $loc) { return $loc }
    }
    return $null
}

function Get-CMakeVersion([string]$bin) {
    try {
        $line = & $bin --version 2>&1 | Select-Object -First 1
        if ($line -match "(\d+\.\d+\.\d+)") { return [Version]$Matches[1] }
    } catch { return $null }
    return $null
}

function Get-GitBin {
    $found = Get-Command git -ErrorAction SilentlyContinue
    if ($found) { return $found.Source }
    foreach ($loc in @(
        "$env:ProgramFiles\Git\cmd\git.exe",
        "$env:ProgramFiles\Git\bin\git.exe"
    )) {
        if (Test-Path $loc) { return $loc }
    }
    return $null
}

# Returns @($generatorString, $installPath) or @($null, $null).
function Get-VSInfo {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        foreach ($range in @("[17.0,18.0)", "[16.0,17.0)")) {
            $path = & $vswhere -latest -version $range `
                -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
                -property installationPath 2>$null
            if ($path) {
                $gen = if ($range -like "*17*") { "Visual Studio 17 2022" } else { "Visual Studio 16 2019" }
                return @($gen, $path)
            }
        }
    }
    # Fallback: check well-known paths without vswhere
    if (Test-Path "$env:ProgramFiles\Microsoft Visual Studio\2022") {
        return @("Visual Studio 17 2022", "$env:ProgramFiles\Microsoft Visual Studio\2022")
    }
    if (Test-Path "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019") {
        return @("Visual Studio 16 2019", "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019")
    }
    return @($null, $null)
}

function Find-Qt([string]$version) {
    if ($QtPath -ne "" -and (Test-Path $QtPath)) { return $QtPath }

    $variants  = @("msvc2022_64", "msvc2019_64")
    $roots     = @("C:\Qt", "D:\Qt", "$env:USERPROFILE\Qt",
                   "$env:ProgramFiles\Qt", "${env:ProgramFiles(x86)}\Qt")
    foreach ($root in $roots) {
        foreach ($vc in $variants) {
            $candidate = "$root\$version\$vc"
            # A valid Qt install has a cmake directory under lib/
            if (Test-Path "$candidate\lib\cmake") { return $candidate }
        }
    }
    return $null
}

function Get-PythonBin {
    foreach ($cmd in @("python", "python3", "py")) {
        $p = Get-Command $cmd -ErrorAction SilentlyContinue
        if ($p) {
            try {
                $ver = & $p.Source --version 2>&1
                if ($ver -match "Python 3\.") { return $p.Source }
            } catch { continue }
        }
    }
    return $null
}

# --- Installation helpers ---

function Install-GitIfMissing {
    Write-Step "Checking Git..."

    if (Get-GitBin) {
        Write-OK "Git found at: $(Get-GitBin)"
        return
    }

    Write-Warn "Git not found. Git is required for SCIRun's Superbuild to download dependencies."

    # Prefer winget (Windows 10 1809+ / Windows 11)
    $winget = Get-Command winget -ErrorAction SilentlyContinue
    if ($winget -and (Confirm-Install "Install Git via winget (recommended)?")) {
        & winget install --id Git.Git --silent --accept-package-agreements --accept-source-agreements
        Sync-EnvPath
        if (Get-GitBin) { Write-OK "Git installed."; return }
        Write-Warn "winget install succeeded but git is not yet on PATH. Continuing..."
        return
    }

    if (Confirm-Install "Download and install Git for Windows directly?") {
        $tmp = "$env:TEMP\git-setup.exe"
        Invoke-Download -url $DL_GIT_URL -dest $tmp -label "Git for Windows"
        Write-Info "Running Git installer (silent)..."
        $p = Start-Process $tmp `
            -ArgumentList "/VERYSILENT /NORESTART /NOCANCEL /SP- /CLOSEAPPLICATIONS /COMPONENTS=icons,ext\reg\shellhere,assoc,assoc_sh" `
            -Wait -PassThru
        Sync-EnvPath
        if ($p.ExitCode -eq 0 -and (Get-GitBin)) { Write-OK "Git installed."; return }
        Exit-WithError "Git installer finished with code $($p.ExitCode).`n    Install manually: https://git-scm.com/download/win"
    }

    Exit-WithError "Git is required.`n    Install from https://git-scm.com/download/win and rerun."
}

function Install-CMakeIfMissing {
    if ($SkipCMakeInstall) { return }
    Write-Step "Checking CMake (need $MIN_CMAKE_VERSION or later)..."

    $bin = Get-CMakeBin
    if ($bin) {
        $ver = Get-CMakeVersion $bin
        if ($ver -and $ver -ge $MIN_CMAKE_VERSION) {
            Write-OK "CMake $ver at $bin"
            $script:cmakeBin = $bin
            return
        }
        Write-Warn "CMake $ver found but $MIN_CMAKE_VERSION is required."
    } else {
        Write-Warn "CMake not found."
    }

    if (-not (Confirm-Install "Download and install CMake $DL_CMAKE_VERSION?")) {
        Exit-WithError "CMake $MIN_CMAKE_VERSION or later is required.`n    Download from https://cmake.org/download/ and rerun."
    }

    if (-not (Test-IsAdmin)) {
        Write-Warn "Not running as Administrator. CMake install may fail or require a UAC prompt."
    }

    $msi = "$env:TEMP\cmake-setup.msi"
    Invoke-Download -url $DL_CMAKE_URL -dest $msi -label "CMake $DL_CMAKE_VERSION"
    Write-Info "Installing CMake (a UAC prompt may appear)..."
    $p = Start-Process msiexec.exe `
        -ArgumentList "/i `"$msi`" /quiet /norestart ADD_CMAKE_TO_PATH=System" `
        -Wait -PassThru
    if ($p.ExitCode -ne 0) {
        Exit-WithError "CMake MSI install failed (code $($p.ExitCode)).`n    Try running this script as Administrator, or install manually: $DL_CMAKE_URL"
    }
    Sync-EnvPath

    $bin = Get-CMakeBin
    if (-not $bin) {
        Exit-WithError "CMake was installed but still not found on PATH.`n    Restart your terminal and rerun this script."
    }
    Write-OK "CMake $DL_CMAKE_VERSION installed."
    $script:cmakeBin = $bin
}

function Install-VSIfMissing {
    if ($SkipVSInstall) { return }
    Write-Step "Checking Visual Studio C++ Build Tools..."

    $gen, $path = Get-VSInfo
    if ($gen) {
        Write-OK "$gen"
        Write-Info "  Location: $path"
        $script:vsGenerator = $gen
        return
    }

    Write-Warn "Visual Studio C++ Build Tools not found."
    Write-Host ""
    Write-Host "    SCIRun requires Visual Studio 2019 or 2022 with the C++ workload." -ForegroundColor Gray
    Write-Host "    The Build Tools download is ~1.5 GB; installation takes 15-30 minutes." -ForegroundColor Gray
    Write-Host "    A progress window will appear during installation." -ForegroundColor Gray
    Write-Host ""

    if (-not (Confirm-Install "Download and install Visual Studio 2022 Build Tools?")) {
        Exit-WithError "Visual Studio C++ Build Tools are required.`n    Download: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022`n    During install, select the 'Desktop development with C++' workload."
    }

    if (-not (Test-IsAdmin)) {
        Write-Warn "Not running as Administrator. You will likely see a UAC prompt."
    }

    $installer = "$env:TEMP\vs_buildtools.exe"
    Invoke-Download -url $DL_VS_BUILDTOOLS_URL -dest $installer -label "VS 2022 Build Tools"

    Write-Info "Launching VS Build Tools installer (this will take 15-30 minutes)..."
    Write-Info "A separate progress window will show download/install status."

    # Workloads:
    #   VCTools         -- core C++ compiler and build tools
    #   Windows11SDK    -- Windows SDK headers/libs
    #   --includeRecommended picks up ATL, CMake integration, etc.
    $vsArgs = "--quiet --wait --norestart " +
              "--add Microsoft.VisualStudio.Workload.VCTools " +
              "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 " +
              "--add Microsoft.VisualStudio.Component.Windows11SDK.22000 " +
              "--includeRecommended"

    $p = Start-Process $installer -ArgumentList $vsArgs -Wait -PassThru

    # 0 = success, 3010 = success but reboot recommended
    if ($p.ExitCode -ne 0 -and $p.ExitCode -ne 3010) {
        Exit-WithError "VS Build Tools installer exited with code $($p.ExitCode).`n    Try installing manually: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022"
    }
    if ($p.ExitCode -eq 3010) {
        Write-Warn "Installation complete, but a system restart is recommended."
        Write-Warn "The build will proceed, but restart if you see unexpected errors."
    }

    $gen, $path = Get-VSInfo
    if (-not $gen) {
        Exit-WithError "VS Build Tools were installed but could not be detected.`n    Restart your terminal and rerun this script."
    }
    Write-OK "Visual Studio 2022 Build Tools installed: $gen"
    $script:vsGenerator = $gen
}

function Install-QtIfMissing {
    if ($SkipQtInstall -or $Headless) { return }
    Write-Step "Checking Qt $QtVersion..."

    $qt = Find-Qt $QtVersion
    if ($qt) {
        Write-OK "Qt found at: $qt"
        $script:qtFoundPath = $qt
        return
    }

    Write-Warn "Qt $QtVersion not found in standard locations."
    Write-Host ""
    Write-Host "    Qt provides SCIRun's graphical interface." -ForegroundColor Gray
    Write-Host "    It will be installed via 'aqtinstall' (a Python-based Qt downloader)." -ForegroundColor Gray
    Write-Host "    Download size: ~1 GB. Python 3 must be installed." -ForegroundColor Gray
    Write-Host ""
    Write-Host "    Alternatives:" -ForegroundColor Gray
    Write-Host "      - Install Qt manually: https://www.qt.io/download-open-source" -ForegroundColor Gray
    Write-Host "        Then rerun with: -QtPath C:\Qt\$QtVersion\msvc2019_64" -ForegroundColor Gray
    Write-Host "      - Build headless (no GUI): -Headless" -ForegroundColor Gray
    Write-Host ""

    if (-not (Confirm-Install "Download and install Qt $QtVersion via aqtinstall?")) {
        Exit-WithError "Qt $QtVersion is required for a GUI build.`n    Supply -QtPath or use -Headless."
    }

    # Ensure Python 3 is available
    $python = Get-PythonBin
    if (-not $python) {
        Write-Warn "Python 3 not found -- needed to run aqtinstall."
        $winget = Get-Command winget -ErrorAction SilentlyContinue
        if ($winget -and (Confirm-Install "Install Python 3.11 via winget?")) {
            & winget install --id Python.Python.3.11 --silent `
                --accept-package-agreements --accept-source-agreements
            Sync-EnvPath
            $python = Get-PythonBin
        }
    }
    if (-not $python) {
        Exit-WithError "Python 3 is required to download Qt.`n    Install from https://www.python.org/downloads/ (check 'Add Python to PATH').`n    Then rerun, or install Qt manually and use -QtPath."
    }

    Write-Info "Installing/upgrading aqtinstall..."
    & $python -m pip install aqtinstall --quiet --upgrade
    if ($LASTEXITCODE -ne 0) {
        Exit-WithError "pip install aqtinstall failed. Check your Python/pip setup."
    }

    $qtInstallRoot = "C:\Qt"
    $qtMajor = [int]($QtVersion.Split(".")[0])
    # Qt 6.5+ ships msvc2022_64; Qt 6.3.x and all Qt 5 use msvc2019_64.
    # msvc2019_64 Qt binaries work fine with VS 2022 (ABI-compatible).
    $qtMinor   = [int]($QtVersion.Split(".")[1])
    $vcVariant = if ($qtMajor -ge 6 -and $qtMinor -ge 5 -and $script:vsGenerator -like "*17*") {
        "win64_msvc2022_64"
    } else {
        "win64_msvc2019_64"
    }

    Write-Info "Downloading Qt $QtVersion ($vcVariant) to $qtInstallRoot..."
    Write-Info "This may take 10-20 minutes depending on your connection speed."
    & $python -m aqt install-qt windows desktop $QtVersion $vcVariant -O $qtInstallRoot
    if ($LASTEXITCODE -ne 0) {
        $dirVariant = $vcVariant -replace "^win64_", ""
        Exit-WithError "Qt download failed.`n    Try installing Qt manually from https://www.qt.io/download-open-source`n    Then rerun with: -QtPath $qtInstallRoot\$QtVersion\$dirVariant"
    }

    # aqtinstall strips the "win64_" prefix when creating the directory
    $dirVariant = $vcVariant -replace "^win64_", ""
    $expected = "$qtInstallRoot\$QtVersion\$dirVariant"
    if (-not (Test-Path $expected)) {
        Exit-WithError "Qt was downloaded but not found at: $expected`n    Rerun with: -QtPath <actual path to the msvc directory>"
    }
    Write-OK "Qt $QtVersion installed at: $expected"
    $script:qtFoundPath = $expected
}

# --- CMake configure ---

function Invoke-Configure([string]$buildDir, [string]$sourceDir, [string]$buildType) {
    Write-Step "Configuring SCIRun Superbuild with CMake..."

    if (-not (Test-Path $buildDir)) {
        Write-Info "Creating build directory: $buildDir"
        New-Item -ItemType Directory -Path $buildDir | Out-Null
    }

    $cmakeParams = @(
        "$sourceDir\Superbuild",
        "-G", $script:vsGenerator,
        "-A", "x64",
        "-DCMAKE_BUILD_TYPE:STRING=$buildType",
        "-DCMAKE_VERBOSE_MAKEFILE:BOOL=$(if ($BuildVerbose) {'ON'} else {'OFF'})",
        "-DBUILD_HEADLESS:BOOL=$(if ($Headless) {'ON'} else {'OFF'})",
        "-DWITH_TETGEN:BOOL=$(if ($WithTetgen) {'ON'} else {'OFF'})",
        "-DBUILD_DOCUMENTATION:BOOL=$(if ($Documentation) {'ON'} else {'OFF'})",
        "-DSCIRUN_QT_MIN_VERSION:STRING=$QtVersion"
    )

    if (-not $Headless -and $script:qtFoundPath -ne "") {
        $cmakeParams += "-DQt_PATH:PATH=$($script:qtFoundPath)"
    }

    if ($CMakeArgs -ne "") {
        # Split on spaces but respect quoted segments
        $extra = $CMakeArgs -split '\s+(?=(?:[^"]*"[^"]*")*[^"]*$)'
        $cmakeParams += $extra
    }

    Write-Info "cmake $cmakeParams"
    Write-Host ""

    Push-Location $buildDir
    try {
        & $script:cmakeBin @cmakeParams
    } finally {
        Pop-Location
    }

    if ($LASTEXITCODE -ne 0) {
        Exit-WithError "CMake configuration failed (exit $LASTEXITCODE).`n`n    Common causes:`n    - Qt path wrong or missing: rerun with -QtPath <path>`n    - VS Build Tools not fully installed: re-run VS installer and select 'Desktop development with C++'`n    - CMake version too old: rerun without -SkipCMakeInstall`n`n    Full output is above. Search for 'CMake Error' lines."
    }

    Write-OK "CMake configuration succeeded."
}

# --- Build ---

function Invoke-Build([string]$buildDir, [string]$buildType, [int]$jobs) {
    Write-Step "Building SCIRun (this takes 30-90 minutes on first build)..."
    Write-Info "Build type: $buildType | Parallel jobs: $jobs"
    Write-Host ""

    $buildParams = @(
        "--build", $buildDir,
        "--config", $buildType,
        "--parallel", "$jobs"
    )
    if ($BuildVerbose) { $buildParams += "--verbose" }

    Write-Info "cmake $buildParams"
    Write-Host ""

    & $script:cmakeBin @buildParams
    if ($LASTEXITCODE -ne 0) {
        Exit-WithError "Build failed (exit $LASTEXITCODE).`n`n    Common causes:`n    - Compiler errors: look for 'error C' lines above`n    - Not enough disk space: need ~15 GB free`n    - Out of memory: try -Jobs 2 to reduce parallelism`n    - Antivirus interference: try temporarily disabling real-time protection`n    - Network errors during dependency downloads: rerun to resume"
    }

    Write-OK "Build succeeded."
}

# --- Post-build DLL deployment ---

function Copy-RuntimeDLL([string]$buildDir, [string]$buildType) {
    if ($Headless) { return }

    Write-Step "Deploying runtime DLLs..."

    $outputDir = "$buildDir\SCIRun\$buildType"
    if (-not (Test-Path $outputDir)) {
        Write-Warn "Output directory not found: $outputDir"
        Write-Warn "DLL copy skipped. Run windeployqt manually if the app doesn't start."
        return
    }

    # Prefer windeployqt -- it resolves the full transitive Qt dependency graph.
    if ($script:qtFoundPath -ne "") {
        $wdq = "$($script:qtFoundPath)\bin\windeployqt.exe"
        if (Test-Path $wdq) {
            $exe = "$outputDir\SCIRun.exe"
            if (Test-Path $exe) {
                Write-Info "Running windeployqt..."
                & $wdq "--$($buildType.ToLower())" "$exe"
                if ($LASTEXITCODE -eq 0) {
                    Write-OK "Qt DLLs deployed via windeployqt."
                } else {
                    Write-Warn "windeployqt returned $LASTEXITCODE. Qt DLLs may be incomplete."
                }
            } else {
                Write-Warn "SCIRun.exe not found at $exe -- skipping windeployqt."
            }
        } else {
            # Manual fallback: copy known Qt DLLs and the platforms plugin
            Write-Info "windeployqt not found; copying Qt DLLs manually..."
            $qtBin = "$($script:qtFoundPath)\bin"
            $major = $QtVersion.Split(".")[0]
            $dlls  = @("Qt${major}Core", "Qt${major}Gui", "Qt${major}Network",
                       "Qt${major}OpenGL", "Qt${major}PrintSupport", "Qt${major}Svg",
                       "Qt${major}Widgets", "Qt${major}Concurrent")
            foreach ($dll in $dlls) {
                $src = "$qtBin\$dll.dll"
                if (Test-Path $src) {
                    Copy-Item $src $outputDir -Force
                    Write-Info "  Copied $dll.dll"
                } else {
                    Write-Warn "  Not found: $src"
                }
            }

            # Qt platform plugin (qwindows.dll) -- required at startup
            $platformsDst = "$outputDir\platforms"
            if (-not (Test-Path $platformsDst)) {
                New-Item -ItemType Directory -Path $platformsDst | Out-Null
            }
            $platformsSrc = "$($script:qtFoundPath)\plugins\platforms"
            if (Test-Path $platformsSrc) {
                Copy-Item "$platformsSrc\*" $platformsDst -Recurse -Force
                Write-OK "Qt platform plugins copied."
            }
        }
    }

    # Qwt (SCIRun's plotting library, built by the Superbuild)
    $qwtDll = "$buildDir\Externals\Build\Qwt_external\src\$buildType\qwt.dll"
    if (Test-Path $qwtDll) {
        Copy-Item $qwtDll $outputDir -Force
        Write-OK "Copied qwt.dll"
    }

    # Python DLL -- version varies, search with a wildcard
    $pythonDlls = Get-ChildItem "$buildDir\Externals\Source\Python_external\PCbuild\amd64\python3*.dll" `
                    -ErrorAction SilentlyContinue
    foreach ($pdll in $pythonDlls) {
        Copy-Item $pdll.FullName $outputDir -Force
        Write-OK "Copied $($pdll.Name)"
    }

    Write-OK "DLL deployment complete."
}

# --- Entry point ---

Write-Banner

# Guard: Windows only
if ($env:OS -ne "Windows_NT") {
    Exit-WithError "This script is for Windows only. On macOS/Linux, use build.sh."
}

# Warn about execution policy (common first-run gotcha)
$policy = Get-ExecutionPolicy -Scope CurrentUser
if ($policy -eq "Restricted" -or $policy -eq "AllSigned") {
    Write-Warn "Your PowerShell execution policy ($policy) may block this script."
    Write-Warn "If you hit an error, open PowerShell as Administrator and run:"
    Write-Warn "  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser"
    Write-Host ""
}

# Resolve paths
$sourceDir = $PSScriptRoot
if (-not $sourceDir -or $sourceDir -eq "") { $sourceDir = (Get-Location).Path }

if ($BuildDir -ne "") {
    $resolvedBuildDir = if ([IO.Path]::IsPathRooted($BuildDir)) {
        $BuildDir
    } else {
        Join-Path $sourceDir $BuildDir
    }
} else {
    $resolvedBuildDir = Join-Path $sourceDir "bin"
}

if ($Debug -and $Release) {
    Exit-WithError "Cannot specify both -Debug and -Release. Omit both for the default Release build."
}
$buildType = if ($Debug) { "Debug" } else { "Release" }

# Resolve Qt version: -Qt6 / -Qt5 switches override -QtVersion; default is Qt 5.15.2
if ($Qt5 -and $Qt6) {
    Exit-WithError "Cannot specify both -Qt5 and -Qt6. Pick one."
}
if ($Qt6) {
    if ($QtVersion -ne "" -and $QtVersion -notlike "6.*") {
        Exit-WithError "-Qt6 conflicts with -QtVersion $QtVersion (not a Qt 6 version).`n    Use -Qt6 alone, or -QtVersion 6.3.1 explicitly."
    }
    if ($QtVersion -eq "") { $QtVersion = "6.3.1" }
} elseif ($Qt5) {
    if ($QtVersion -ne "" -and $QtVersion -notlike "5.*") {
        Exit-WithError "-Qt5 conflicts with -QtVersion $QtVersion (not a Qt 5 version).`n    Use -Qt5 alone, or -QtVersion 5.15.2 explicitly."
    }
    if ($QtVersion -eq "") { $QtVersion = "5.15.2" }
} elseif ($QtVersion -eq "") {
    $QtVersion = "5.15.2"
}

$numJobs = if ($Jobs -gt 0) {
    $Jobs
} else {
    [int]([Environment]::GetEnvironmentVariable("NUMBER_OF_PROCESSORS") ?? "4")
}

# Print summary
Write-Host "  Source dir   : $sourceDir" -ForegroundColor White
Write-Host "  Build dir    : $resolvedBuildDir" -ForegroundColor White
Write-Host "  Build type   : $buildType" -ForegroundColor White
Write-Host "  Parallel jobs: $numJobs" -ForegroundColor White
Write-Host "  Qt version   : $QtVersion$(if ($Headless) {' (N/A -- headless build)'})" -ForegroundColor White
Write-Host "  Headless     : $Headless" -ForegroundColor White
Write-Host "  With Tetgen  : $WithTetgen" -ForegroundColor White
Write-Host ""

# Verify the Superbuild is where we expect it
if (-not (Test-Path "$sourceDir\Superbuild\CMakeLists.txt")) {
    Exit-WithError "Cannot find Superbuild\CMakeLists.txt under: $sourceDir`n    Run this script from the SCIRun repository root, or check out the repository first."
}

# Install / verify prerequisites
Install-GitIfMissing
Install-CMakeIfMissing
Install-VSIfMissing
Install-QtIfMissing

# Final verification pass before we start building
Write-Step "Verifying all prerequisites..."

$cmakeBin = Get-CMakeBin
if (-not $cmakeBin) {
    Exit-WithError "CMake not found after installation attempt. Restart your terminal and rerun."
}
$script:cmakeBin = $cmakeBin
$cmakeVer = Get-CMakeVersion $cmakeBin
if (-not $cmakeVer -or $cmakeVer -lt $MIN_CMAKE_VERSION) {
    Exit-WithError "CMake $MIN_CMAKE_VERSION or later required (found: $cmakeVer).`n    Download from https://cmake.org/download/"
}
Write-OK "CMake $cmakeVer"

if (-not (Get-GitBin)) {
    Exit-WithError "Git not found. Install from https://git-scm.com/download/win"
}
Write-OK "Git"

$gen, $vsPath = Get-VSInfo
if (-not $gen) {
    Exit-WithError "Visual Studio C++ Build Tools not found.`n    Install from: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022`n    Select 'Desktop development with C++' workload."
}
$script:vsGenerator = $gen
Write-OK "$gen"

if (-not $Headless) {
    $qt = Find-Qt $QtVersion
    if (-not $qt) {
        Exit-WithError "Qt $QtVersion not found.`n    Options:`n    1. Rerun with: -QtPath C:\Qt\$QtVersion\msvc2019_64`n    2. Install Qt from https://www.qt.io/download-open-source`n    3. Build without GUI: -Headless"
    }
    $script:qtFoundPath = $qt
    Write-OK "Qt $QtVersion at $qt"
}

Write-Host ""
Write-Host "  All prerequisites satisfied. Starting build..." -ForegroundColor Green

# Configure, build, deploy
Invoke-Configure -buildDir $resolvedBuildDir -sourceDir $sourceDir -buildType $buildType
Invoke-Build     -buildDir $resolvedBuildDir -buildType $buildType -jobs $numJobs
Copy-RuntimeDLL  -buildDir $resolvedBuildDir -buildType $buildType

# Done
$bar = "=" * 70
Write-Host ""
Write-Host $bar -ForegroundColor Green
Write-Host "  SCIRun build complete!" -ForegroundColor Green
Write-Host ""
$exe = "$resolvedBuildDir\SCIRun\$buildType\SCIRun.exe"
if (Test-Path $exe) {
    Write-Host "  Executable: $exe" -ForegroundColor Green
} else {
    Write-Host "  Output: $resolvedBuildDir\SCIRun\$buildType\" -ForegroundColor Green
}
Write-Host $bar -ForegroundColor Green
Write-Host ""

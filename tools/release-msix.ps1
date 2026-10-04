# Usage (PowerShell, on Windows, from the repo root):
#   $env:COWS_MSIX_IDENTITY  = '<Package/Identity/Name from Partner Center>'
#   $env:COWS_MSIX_PUBLISHER = '<Package/Identity/Publisher, e.g. CN=...>'
#   $env:COWS_MSIX_PUBLISHER_NAME = '<Package/Properties/PublisherDisplayName>'
#   powershell -ExecutionPolicy Bypass -File tools\release-msix.ps1 [-Arch x64,arm64] [-Certificate cert.pfx -Password pw]
#
# For each architecture (both by default: the Store submission ships x64 and
# arm64), builds the Release exe if it is not built yet (tools\build-windows.bat
# with COWS_ARCH), lays out the package (exe, AppxManifest.xml filled in,
# Assets) and packs Deploy\Microsoft-Store\out\CowsInLove-<version>-<arch>.msix
# with makeappx. Upload those .msix to Partner Center, which signs them for the
# Store. A certificate is only for sideloading a test build; its subject must
# equal COWS_MSIX_PUBLISHER.
param(
    [ValidateSet('x64', 'arm64')] [string[]]$Arch = @('x64', 'arm64'),
    [string]$Certificate = '',
    [string]$Password = ''
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

function Require([string]$name) {
    $value = [Environment]::GetEnvironmentVariable($name)
    if (-not $value) { throw "set $name (see the top of tools\release-msix.ps1)" }
    return $value
}

$identity = Require 'COWS_MSIX_IDENTITY'
$publisher = Require 'COWS_MSIX_PUBLISHER'
$publisherName = Require 'COWS_MSIX_PUBLISHER_NAME'

$project = Get-Content CMakeLists.txt | Select-String 'project\(Cows VERSION ([0-9.]+)'
$version = "$($project.Matches[0].Groups[1].Value).0"

$kits = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\bin'
$sdk = Get-ChildItem $kits -Directory | Where-Object { $_.Name -match '^10\.' } |
    Sort-Object { [version]$_.Name } -Descending |
    Where-Object { Test-Path (Join-Path $_.FullName 'x64\makeappx.exe') } |
    Select-Object -First 1
if (-not $sdk) { throw 'no Windows SDK with makeappx.exe: install the Windows 10/11 SDK' }
$makeappx = Join-Path $sdk.FullName 'x64\makeappx.exe'
$signtool = Join-Path $sdk.FullName 'x64\signtool.exe'
$makepri = Join-Path $sdk.FullName 'x64\makepri.exe'

$out = 'Deploy\Microsoft-Store\out'
New-Item -ItemType Directory -Force -Path $out | Out-Null

function Pack([string]$arch) {
    $build = if ($arch -eq 'x64') { 'build-windows' } else { "build-windows-$arch" }
    $exe = "$build\Apps\CowsInLove\Cows.exe"
    if (-not (Test-Path $exe)) {
        $env:COWS_ARCH = $arch
        cmd /c 'tools\build-windows.bat Release'
        if ($LASTEXITCODE -ne 0) { throw "the $arch Release build failed" }
    }

    $layout = Join-Path $out "package-$arch"
    if (Test-Path $layout) { Remove-Item -Recurse -Force $layout }
    New-Item -ItemType Directory -Force -Path $layout | Out-Null

    Copy-Item $exe $layout
    Copy-Item -Recurse 'Deploy\Microsoft-Store\Assets' (Join-Path $layout 'Assets')

    (Get-Content 'Deploy\Microsoft-Store\AppxManifest.xml' -Raw) `
        -replace '@IDENTITY_NAME@', $identity `
        -replace '@PUBLISHER@', [Security.SecurityElement]::Escape($publisher) `
        -replace '@PUBLISHER_DISPLAY_NAME@', [Security.SecurityElement]::Escape($publisherName) `
        -replace '@VERSION@', $version `
        -replace '@ARCH@', $arch |
        Set-Content -Encoding UTF8 (Join-Path $layout 'AppxManifest.xml')

    # resources.pri maps Assets\X.png to its scale-100/200 and targetsize files.
    $config = Join-Path $out "priconfig-$arch.xml"
    & $makepri createconfig /cf $config /dq en-US /pv 10.0.0 /o | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'makepri createconfig failed' }
    # One .msix per architecture, not a bundle: keep every scale in
    # resources.pri rather than splitting resource packs that nothing installs.
    (Get-Content $config -Raw) -replace '(?s)\s*<packaging>.*?</packaging>', '' |
        Set-Content -Encoding UTF8 $config
    & $makepri new /pr $layout /cf $config /mn (Join-Path $layout 'AppxManifest.xml') `
        /of (Join-Path $layout 'resources.pri') /o
    if ($LASTEXITCODE -ne 0) { throw 'makepri failed' }

    $msix = Join-Path $out "CowsInLove-$version-$arch.msix"
    & $makeappx pack /o /d $layout /p $msix
    if ($LASTEXITCODE -ne 0) { throw 'makeappx failed' }

    if ($Certificate) {
        & $signtool sign /fd SHA256 /a /f $Certificate /p $Password $msix
        if ($LASTEXITCODE -ne 0) { throw 'signtool failed' }
    }

    Write-Output $msix
}

foreach ($a in $Arch) { Pack $a }

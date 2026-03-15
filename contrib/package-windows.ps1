param(
	[string] $RepoRoot = (Split-Path -Parent $PSScriptRoot),
	[string] $MsysRoot = "C:\msys64",
	[string] $OutDir = "",
	[switch] $Zip
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path $RepoRoot).Path
if ([string]::IsNullOrWhiteSpace($OutDir)) {
	$OutDir = Join-Path $RepoRoot "dist\windows-portable"
}

$bashExe = Join-Path $MsysRoot "usr\bin\bash.exe"
$exePath = Join-Path $RepoRoot "pianobar.exe"
$configExample = Join-Path $RepoRoot "contrib\config-example"
$copyingPath = Join-Path $RepoRoot "COPYING"
$readmePath = Join-Path $RepoRoot "README.rst"
$certBundlePath = Join-Path $MsysRoot "mingw64\ssl\certs\ca-bundle.crt"

if (!(Test-Path $bashExe)) {
	throw "MSYS2 bash not found at $bashExe"
}
if (!(Test-Path $exePath)) {
	throw "Expected build artifact not found at $exePath. Build pianobar.exe first."
}

New-Item -ItemType Directory -Force $OutDir | Out-Null

$msysRepoPath = "/" + ($RepoRoot.Substring(0, 1).ToLower()) + $RepoRoot.Substring(2).Replace("\", "/")
$msysExePath = "$msysRepoPath/pianobar.exe"

$lddScript = @"
export MSYSTEM=MINGW64
export CHERE_INVOKING=1
source /etc/profile
cd '$msysRepoPath'
ldd '$msysExePath' | sed -n 's#.*=> \(/mingw64/bin/[^ ]*\.dll\) .*#\1#p' | sort -u
"@

$dlls = & $bashExe -lc $lddScript
if ($LASTEXITCODE -ne 0) {
	throw "Failed to inspect DLL dependencies with ldd."
}

Copy-Item $exePath $OutDir -Force
Copy-Item $configExample (Join-Path $OutDir "config-example") -Force
Copy-Item $copyingPath $OutDir -Force
Copy-Item $readmePath $OutDir -Force

if (Test-Path $certBundlePath) {
	Copy-Item $certBundlePath (Join-Path $OutDir "ca-bundle.crt") -Force
}

foreach ($dll in $dlls) {
	$windowsDll = Join-Path $MsysRoot ($dll.TrimStart("/") -replace "/", "\")
	if (Test-Path $windowsDll) {
		Copy-Item $windowsDll $OutDir -Force
	}
}

$launcher = @'
@echo off
setlocal
set "ROOT=%~dp0"
set "PATH=%ROOT%;%PATH%"
cd /d "%ROOT%"
if not exist "%APPDATA%\pianobar" mkdir "%APPDATA%\pianobar" >NUL 2>NUL
if not exist "%APPDATA%\pianobar\config" (
	echo Copy "config-example" to "%APPDATA%\pianobar\config" and edit it before first use.
)
.\pianobar.exe
'@
Set-Content -Path (Join-Path $OutDir "run-pianobar.cmd") -Value $launcher

$notes = @'
Portable Windows package for pianobar

Files:
- run-pianobar.cmd: preferred launcher on Windows
- pianobar.exe: the application
- config-example: starter config
- ca-bundle.crt: TLS root certificates

First-time setup:
1. Copy config-example to %APPDATA%\pianobar\config
2. Edit %APPDATA%\pianobar\config with your settings
3. If needed, set: ca_bundle = ./ca-bundle.crt

Run:
- Double-click run-pianobar.cmd
- or run pianobar.exe from cmd/PowerShell in this folder
'@
Set-Content -Path (Join-Path $OutDir "README-Windows.txt") -Value $notes

if ($Zip) {
	$zipPath = Join-Path (Split-Path $OutDir -Parent) "pianobar-windows-portable.zip"
	if (Test-Path $zipPath) {
		Remove-Item $zipPath -Force
	}
	$zipCreated = $false
	for ($attempt = 1; $attempt -le 5 -and -not $zipCreated; ++$attempt) {
		try {
			Compress-Archive -Path (Join-Path $OutDir "*") -DestinationPath $zipPath
			$zipCreated = $true
		} catch {
			if ($attempt -eq 5) {
				throw
			}
			Start-Sleep -Seconds 2
		}
	}
	Write-Host "Created zip: $zipPath"
}

Write-Host "Portable package created at: $OutDir"

$ErrorActionPreference='Stop'
$dll=Join-Path $PSScriptRoot 'client\TavernDuels'
$src=Join-Path $PSScriptRoot 'window-source'
& "$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /target:winexe /platform:x64 /main:TavernLive "/out:$dll\TavernLive.exe" /reference:System.Windows.Forms.dll /reference:System.Drawing.dll /reference:System.Web.Extensions.dll "/reference:$dll\Microsoft.Web.WebView2.Core.dll" "/reference:$dll\Microsoft.Web.WebView2.WinForms.dll" "$src\TavernWindow.cs" "$src\TavernLive.cs"
if($LASTEXITCODE -ne 0){throw 'Tavern window build failed.'}

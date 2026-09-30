param([Parameter(Mandatory=$true)][string]$ClientPath)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $ClientPath).Path
if(!(Test-Path -LiteralPath (Join-Path $root 'eqgame.exe'))){throw 'Select your compatible RoF2 client directory.'}
if(Get-Process eqgame -ErrorAction SilentlyContinue){throw 'Close EverQuest first.'}
$ui=Join-Path $root 'uifiles\default'
$equi=Join-Path $ui 'EQUI.xml'
$animations=Join-Path $ui 'EQUI_Animations.xml'
$text=[IO.File]::ReadAllText($equi)
$a=[IO.File]::ReadAllText($animations)
if($text -notmatch '</Composite>'){throw 'Unexpected EQUI.xml; no files changed.'}
$start=$a.IndexOf('<Ui2DAnimation item="A_DragItem">')
if($start -lt 0){throw 'Missing A_DragItem animation.'}
$end=$a.IndexOf('</Ui2DAnimation>',$start)
$section=$a.Substring($start,$end-$start)
if($section -notmatch 'dragitem223.tga'){
 if(([regex]::Matches($section,'<Frames>')).Count -ne 222){throw 'Custom icon atlas layout: merge dragitem223 at frame 223 manually before installing.'}
 $frame='<Frames><Texture>dragitem223.tga</Texture><Location><X>0</X><Y>0</Y></Location><Size><CX>256</CX><CY>256</CY></Size><Hotspot><X>0</X><Y>0</Y></Hotspot><Duration>1000</Duration></Frames>'
 $a=$a.Insert($end,$frame)
}
if($a -notmatch '<TextureInfo item="dragitem223.tga">'){
 $a=$a.Replace('<Ui2DAnimation item="A_DragItem">','<TextureInfo item="dragitem223.tga"><Size><CX>256</CX><CY>256</CY></Size></TextureInfo><Ui2DAnimation item="A_DragItem">')
}
if($text -notmatch '<Include>\s*EQUI_NMSSpellShardWnd.xml\s*</Include>'){
 $text=$text.Replace('</Composite>',"<Include>EQUI_NMSSpellShardWnd.xml</Include>`r`n</Composite>")
}
$null=[xml]$a;$null=[xml]$text
$backup=Join-Path $root ('feature-backup-'+(Get-Date -Format yyyyMMdd-HHmmss))
New-Item -ItemType Directory -Path $backup | Out-Null
$payload=Join-Path $PSScriptRoot 'client-files'
foreach($file in Get-ChildItem -LiteralPath $payload -Recurse -File){
 $rel=$file.FullName.Substring($payload.Length+1);$dst=Join-Path $root $rel
 if(Test-Path -LiteralPath $dst){$save=Join-Path $backup $rel;New-Item -ItemType Directory -Force (Split-Path $save) | Out-Null;Copy-Item -LiteralPath $dst -Destination $save}
 New-Item -ItemType Directory -Force (Split-Path $dst) | Out-Null
 Copy-Item -LiteralPath $file.FullName -Destination $dst -Force
}
Copy-Item -LiteralPath $equi -Destination (Join-Path $backup 'EQUI.xml')
Copy-Item -LiteralPath $animations -Destination (Join-Path $backup 'EQUI_Animations.xml')
[IO.File]::WriteAllText($equi,$text,[Text.Encoding]::ASCII)
[IO.File]::WriteAllText($animations,$a,[Text.Encoding]::ASCII)
Write-Output "Installed. Backup: $backup. Log in and use /spellshards."

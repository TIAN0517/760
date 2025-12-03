$path = $null
if ($args.Count -gt 0) {
    $path = $args[0]
}
if ([string]::IsNullOrWhiteSpace($path)) {
    Write-Host "USAGE: check_string.ps1 <path-to-dll>"
    return
}
$path = $path.Trim()
if (-not (Test-Path $path)) {
    Write-Host "FILE NOT FOUND: $path"
    return
}
$bytes = [System.IO.File]::ReadAllBytes($path)
$needle = [Text.Encoding]::ASCII.GetBytes("InjectPlaintextToBDBuffer head32")
$found = $false
for ($i = 0; $i -le $bytes.Length - $needle.Length; $i++) {
    $match = $true
    for ($j = 0; $j -lt $needle.Length; $j++) {
        if ($bytes[$i + $j] -ne $needle[$j]) {
            $match = $false
            break
        }
    }
    if ($match) {
        $found = $true
        break
    }
}
if ($found) {
    Write-Host "FOUND"
} else {
    Write-Host "NOT FOUND"
}

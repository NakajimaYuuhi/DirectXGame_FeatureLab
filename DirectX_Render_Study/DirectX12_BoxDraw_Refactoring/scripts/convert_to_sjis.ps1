param(
    [string]$targetPath = "."
)

$exts = @('.cpp', '.h', '.hpp', '.c', '.inl', '.hlsl', '.hlsli', '.fx')
$utf8 = [System.Text.Encoding]::UTF8
$cp932 = [System.Text.Encoding]::GetEncoding(932)

function Convert-SingleFile([string]$path) {
    if (-not (Test-Path $path)) { return }
    $bytes = [System.IO.File]::ReadAllBytes($path)
    if ($bytes.Length -eq 0) { return }

    $hasBom = ($bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF)
    $contentBytes = if ($hasBom) { $bytes[3..($bytes.Length - 1)] } else { $bytes }

    try {
        $text = $utf8.GetString($contentBytes)
        $utf8JpMatches = [regex]::Matches($text, '[\u3040-\u30ff\u4e00-\u9fff]')
        
        $strCp932 = $cp932.GetString($contentBytes)
        $cp932JpMatches = [regex]::Matches($strCp932, '[\u3040-\u30ff\u4e00-\u9fff]')

        if ($utf8JpMatches.Count -gt 0 -and ($hasBom -or $cp932JpMatches.Count -lt $utf8JpMatches.Count)) {
            $normalized = $text.Replace([char]0x301c, [char]0xff5e)
            $sjisBytes = $cp932.GetBytes($normalized)
            [System.IO.File]::WriteAllBytes($path, $sjisBytes)
            Write-Output "[CONVERTED] UTF-8 -> Shift_JIS: $path"
            return
        }
    } catch {}

    if ($hasBom) {
        [System.IO.File]::WriteAllBytes($path, $contentBytes)
        Write-Output "[STRIPPED BOM] Shift_JIS kept: $path"
    } else {
        Write-Output "[ALREADY SJIS] $path"
    }
}

if (Test-Path $targetPath -PathType Leaf) {
    Convert-SingleFile $targetPath
} elseif (Test-Path $targetPath -PathType Container) {
    Write-Output "=== Scanning directory: $targetPath ==="
    Get-ChildItem -Path $targetPath -Recurse -File | Where-Object {
        $exts -contains $_.Extension.ToLower() -and $_.FullName -notmatch '\\(\.git|\.vs|build|\.agents|bin|obj)\\'
    } | ForEach-Object {
        Convert-SingleFile $_.FullName
    }
} else {
    Write-Error "Target path not found: $targetPath"
}

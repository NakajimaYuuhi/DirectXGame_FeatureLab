param(
    [string]$targetPath = "Source"
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$pyScript = Join-Path $scriptDir "to_sjis.py"

if (Test-Path $pyScript) {
    python $pyScript $targetPath
} else {
    $rootDir = Split-Path -Parent $scriptDir
    $rootPy = Join-Path $rootDir "to_sjis.py"
    if (Test-Path $rootPy) {
        python $rootPy $targetPath
    } else {
        Write-Error "to_sjis.py not found."
    }
}

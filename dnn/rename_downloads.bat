@echo off
setlocal

echo changing files

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$files = Get-ChildItem -Path 'dnn' -Recurse -Include '*_data.c', '*_data.h' | Where-Object { $_.Name -notmatch 'dump_data' }; " ^
    "foreach ($file in $files) { " ^
    "    (Get-Content $file.FullName) | ForEach-Object { " ^
    "        $_ -creplace 'OPUS', 'OAC' " ^
    "           -creplace 'Opus', 'Oac' " ^
    "           -creplace 'opus_', 'oac_' " ^
    "           -creplace 'opus', 'oac' " ^
    "    } | Set-Content -Path $file.FullName " ^
    "}"

echo Done.
pause

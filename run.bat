@echo off
set TIMESTAMP=%date:~0,4%%date:~5,2%%date:~8,2%-%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%
set OUTFILE=result-%TIMESTAMP%.txt

echo Output: %OUTFILE%
profanity.exe --matching profanity.txt --suffix-count 4 --quit-count 1 --skip 1 -o %OUTFILE%
pause

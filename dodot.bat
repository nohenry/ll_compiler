@echo off

FOR %%I IN (*.dot) DO "C:\Program Files\Graphviz\bin\dot" -Tsvg %%I -o %%~nI.svg
FOR %%I IN (*.dot) DO del %%I

set output=task1
del %output%.exe
gcc task1.c -Wall -o %output% -std=c99 -save-temps
%output%.exe

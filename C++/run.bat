set output=task1
del %output%.exe
gcc -Wall task1.cpp CReport.cpp -o %output% -save-temps -lstdc++ -m32
%output%.exe

# with gnuplot: load "xyz.plt"
set xlabel "X"
set ylabel "Y"
set zlabel "Z"
set xrange [-1:1]
set yrange [-1:1]
set zrange [-1:1]
splot "xyz.txt" with lines
pause 1
reread

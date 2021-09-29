set multiplot
set xrange [0.0:100]
set yrange [-0.1:1.1]
set size 1,0.09
set origin 0,0
plot "f.data" u 1:2 t "feed belt" lc rgb "blue" w l
set origin 0,0.09
plot "f.data" u 1:3 t "bottle at feed belt" lc rgb "blue" w l
set origin 0,0.18
plot "f.data" u 1:4 t "outlet belt" lc rgb "blue" w l
set origin 0,0.27
plot "f.data" u 1:5 t "bottle at outlet belt" lc rgb "blue" w l
set origin 0,0.36
plot "f.data" u 1:6 t "pusher" lc rgb "blue" w l
set origin 0,0.45
plot "f.data" u 1:7 t "stopper" lc rgb "blue" w l
set origin 0,0.54
plot "f.data" u 1:8 t "filler cylinder" lc rgb "blue" w l
set origin 0,0.63
plot "f.data" u 1:9 t "closing cylinder" lc rgb "blue" w l
set origin 0,0.72
plot "f.data" u 1:10 t "heating" lc rgb "blue" w l
set origin 0,0.81
plot "f.data" u 1:11 t "nozzle valve" lc rgb "blue" w l
set origin 0,0.90
plot "f.data" u 1:12 t "bottle full" lc rgb "blue" w l
pause 3
reread

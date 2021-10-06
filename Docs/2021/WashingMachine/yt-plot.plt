clear
set multiplot

set origin 0,0.81
set size 1,0.2
set yrange [0:20]
plot "yt-plot.dat" u 1:8 t "H2O Pegel" lc rgb "blue" w l

set origin 0,0.70
set size 1,0.13
set yrange [0:1.2]
plot "yt-plot.dat" u 1:6 t "H2O Einlauf" lc rgb "blue" w l

set origin 0,0.59
set size 1,0.13
set yrange [0:1.2]
plot "yt-plot.dat" u 1:7 t "Pumpe" lc rgb "blue" w l

set origin 0,0.48
set size 1,0.13
set yrange [0:1.2]
plot "yt-plot.dat" u 1:4 t "Heizung" lc rgb "blue" w l

set origin 0,0.29
set size 1,0.21
set yrange [15:35]
plot "yt-plot.dat" u 1:5 t "Temperatur" lc rgb "blue" w l

set origin 0,0.12
set size 1,0.19
set yrange [0:60]
plot "yt-plot.dat" u 1:10 t "Waschmittel" lc rgb "blue" w l

set origin 0,0.0
set size 1,0.14
set yrange [-1000:1000]
plot "yt-plot.dat" u 1:11 t "Drehzahl" lc rgb "blue" w l

unset multiplot

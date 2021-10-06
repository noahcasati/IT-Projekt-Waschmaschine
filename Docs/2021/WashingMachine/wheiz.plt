set yrange [0:70]
set title("Water Temperature");
set xlabel("sec");
set ylabel("°C");
set grid;
plot "wheiz.dat" using 1:5 with lines t "Temperature"

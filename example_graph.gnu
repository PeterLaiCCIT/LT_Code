set term post eps enhanced "Times" 22
set output "example_graph.eps"

set grid

set xlabel "K [input symbols]"
set ylabel "Decoding time [{/Symbol m}sec]"

plot "complexity_GF2.txt" using 1:2 notitle with lp pt 71 ps 1.8 lw 3



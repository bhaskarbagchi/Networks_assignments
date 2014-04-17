set pm3d
unset surface
set terminal png enhanced large

set xlabel '-log(p)'
set ylabel 'Window size'
set zlabel 'Throughput'

set output 'selective_repeat_plot.png'
set title 'Selective-Repeat ARQ'
splot 'results_selective_b.txt'

set output 'go_back_n_plot.png'
set title 'Go-Back-N ARQ'
splot 'results_goBack_N_b.txt'

set output 'one_bit_sliding_plot.png'
set title 'One bit Sliding Window'
splot 'results_sliding_b.txt'
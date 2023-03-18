use v5.14;
use strict;
use warnings;

my @params = (
    1.0/8,  # 0.125
    .50/3 , # 0.1666
    3.0/16, # 0.1875
    # ---------------
    1.0/4 , # 0.25
    1.0/3 , # 0.3333
    3.0/8 , # 0.375
    # ---------------
    1.0/2 , # 0.5
    2.0/3 , # 0.6666
    3.0/4 , # 0.75
    # ---------------
    1.0
);

for (my $i=0; $i<scalar(@params); $i++) {
    my $fs = 48_000;
    my $bpm = 120;
    my $total_time = 3;

    my $samples = $fs / (($bpm / 60) / $params[$i]);
    my $n = ($fs * $total_time) / $samples;
    my $gain = 2 ** (log2(0.001) / $n);
    say sprintf("[%d] n:%4.1f", $i, $n), " gain: ", $gain;

    my $m = ($total_time * ($bpm / 60)) / $params[$i];
    say sprintf("[%d] m:%4.1f", $i, $m), " ", (log2(0.001) / $m);
}

sub log2 {
    return log($_[0]) / log(2.0);
}

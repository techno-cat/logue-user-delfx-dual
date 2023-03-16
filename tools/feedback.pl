use v5.14;
use strict;
use warnings;

{
    my $fs = 48_000;

    my $n = 16;
    my $gain = 2 ** (log2(0.001) / $n);

    say "gain: ", $gain;
}

sub log2 {
    return log($_[0]) / log(2.0);
}

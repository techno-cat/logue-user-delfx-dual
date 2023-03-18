use v5.14;
use strict;
use warnings;

use constant TABLE_SIZE => 0x100;

my @table = map {
    my $val = 2 ** ($_ / TABLE_SIZE);
    my $fixed_value = to_fixed_value($val);
    my $tmp = $fixed_value / 0x100_0000;
    #printf("%.5f, diff: %f\n", $val, $val - $tmp);
    $fixed_value;
} 0..TABLE_SIZE;

# test
my @test_values = (
    # -1.0, -1/256, -64/256, -32/256, -1/256,0, 0.25, 1.0, 10.0
    -2.0, -1.2345, -0.9, -0.4, -0.01, -0.001, 0, 0.01, 0.4, 0.9, 1.2345, 2.0
);
foreach my $val ( @test_values ) {
    my $res1 = my_pow2_ex( $val ) >> 8;
    my $res2 = 2 ** $val;
    my $diff = $res2 - ($res1 / 0x1_0000);

    if ( abs($diff) < 0.0001 ) {
        printf( "pass! log2(%f) = %.6f, %.6f, diff: %.8f\n",
            $val, $res2, $res1 / 0x1_0000, $res2 - ($res1 / 0x1_0000) );
    }
    else {
        printf( "failed! log2(%f) = %.6f, %.6f, diff: %.8f\n",
            $val, $res2, $res1 / 0x1_0000, $res2 - ($res1 / 0x1_0000) );
    }
}

#dump_table( @table );

sub my_pow2_ex {
    my $val = shift;

    if ( $val < 0 ) {
        # Cであれば補数表現されたものを、算術シフトとビットマスクで処理できる
        my $i = int($val);
        my $frac = int( abs($val - $i) * 0x1_0000 );
        if ( 0 < $frac ) {
            $i--;
            $frac = 0x1_0000 - $frac;
        }

        my $j = $frac >> 8;
        my $ratio = $frac & 0x00FF;
        my $delta = $table[$j+1] - $table[$j];
        my $ret = $table[$j] + (($delta * $ratio) >> 8);
    #say "val=", $val, ", frac=", sprintf("0x%04X",$frac), ", delta=", sprintf("0x%08X",$delta);
        return $ret >> (-$i);
    }
    else {
        my $i = int($val);
        my $frac = int( abs($val - $i) * 0x1_0000 );

        my $j = $frac >> 8;
        my $ratio = $frac & 0x00FF;
        my $delta = $table[$j+1] - $table[$j];
        my $ret = $table[$j] + (($delta * $ratio) >> 8);
    #say "val=", $val, ", frac=", sprintf("0x%04X",$frac), ", delta=", sprintf("0x%08X",$delta);
        return $ret << $i;
    }
}

sub dump_table {
    my @copied = @_;
    while ( my @tmp = splice(@copied, 0, 4) ) {
        my $str = join(', ', map {
            sprintf("0x%08X", $_);
        } @tmp);

        $str .= ',' if ( @copied );
        say $str;
    }
}

sub to_fixed_value {
    return int($_[0] * 0x100_0000);
}

sub log2 {
    return log($_[0]) / log(2.0);
}

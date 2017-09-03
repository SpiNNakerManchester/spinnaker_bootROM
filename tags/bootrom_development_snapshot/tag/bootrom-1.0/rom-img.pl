#!/usr/bin/perl

# Read a hex file (1 word/line) and write a binary text
# file suitable for Artisan ROM generator

use strict;
use warnings;

my $size = 8192;
my $bits = 32;
my $digits = $bits / 4;

my $count = 0;

while (<>)
{
    chomp;
    die "!! $. $_\n" unless /^[0-9A-Fa-f]{$digits}$/;

    my $v = hex $_;
    my $p = '';

    for (my $i = $bits-1; $i >=0; $i--)
    {
	$p .= ($v & (1 << $i)) ? '1' : '0';
    }

    print "$p\n";
    $count++;
}

my $free = $size - $count;

warn "$count words in ROM ($free free)\n";

for (my $i = $count; $i < $size; $i++)
{
    print '1' x $bits, "\n";
}

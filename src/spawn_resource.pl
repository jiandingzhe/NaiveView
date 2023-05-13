#!/usr/bin/perl

use strict;
use feature qw/say/;
use File::Path qw/mkpath/;
use File::Basename;
use File::Spec::Functions;

my ($f_in, $d_out, $sym_prefix_out, $namespace) = @ARGV;


die "input file not specified" if !defined $f_in;
die "input file <$f_in> does not exist" if !-f $f_in;
die "output directory not specified" if !defined $d_out;
die "output file prefix not specified" if !defined $sym_prefix_out;

my $symbol = $sym_prefix_out;
$symbol =~ s/[^A-Za-z0-9_\-]/_/g;

my $full_prefix = catfile $d_out, $sym_prefix_out;

# create output directory
my $out_parent = dirname $full_prefix;
mkpath $out_parent;
die "cannot create output directory $out_parent" if !-d $out_parent;

# open input file, get file properties
open my $fh_in, '<', $f_in or die "failed to open input file \"$f_in\": $!";
binmode $fh_in or die "failed to set input file handle to binmode: $!";

seek $fh_in, 0, 2;
my $file_size = tell $fh_in;
seek $fh_in, 0, 0;


#
# write header
#
my $f_hdr = $full_prefix.'.h';
open my $fh_hdr, '>', $f_hdr or die "failed to open output header file $f_hdr: $!";

print $fh_hdr <<HEREDOC;
// automatically generated file, do not modify
#pragma once

#include <cstddef>
#include <cstdint>

HEREDOC

print $fh_hdr <<HEREDOC if length($namespace);
namespace $namespace
{
HEREDOC

print $fh_hdr <<HEREDOC;
static const size_t ${symbol}_size = $file_size;
extern const std::int8_t ${symbol}_data[];
HEREDOC

print $fh_hdr <<HEREDOC if length($namespace) > 0;
} // namespace $namespace
HEREDOC

close $fh_hdr;

#
# write source
#
my $f_src = $full_prefix.'.cpp';
open my $fh_src, '>', $f_src or die "failed to open output source file $f_src: $!";
my $header_inc_name = basename $f_hdr;
print $fh_src <<HEREDOC;
// automatically generated file, do not modify
#include "$header_inc_name"

HEREDOC

print $fh_src <<HEREDOC if length($namespace) > 0;
namespace $namespace
{
HEREDOC

print $fh_src <<HEREDOC;
const std::int8_t ${symbol}_data[] = {
HEREDOC

while (1)
{
    my $data;
    my $num_got = read $fh_in, $data, 20;
    last if $num_got == 0;
    my @data = unpack 'c*', $data;
    
    my $concat = join ', ', @data;
    
    print $fh_src <<HEREDOC;
    $concat,
HEREDOC
}

print $fh_src <<HEREDOC;
    0
};
HEREDOC

print $fh_src <<HEREDOC if length($namespace) > 0;
} // namespace $namespace
HEREDOC

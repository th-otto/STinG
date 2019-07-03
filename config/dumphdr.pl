#!/usr/bin/perl -w

use strict;

my $me = "dumphdr.pl";


sub isascii($)
{
	my ($c) = @_;
	return $c >= 32 && $c <= 126;
}


sub idstr($)
{
	my ($id) = @_;
	my $c1 = ($id >> 24) & 0xff;
	my $c2 = ($id >> 16) & 0xff;
	my $c3 = ($id >> 8) & 0xff;
	my $c4 = ($id >> 0) & 0xff;
	if (isascii($c1) && isascii($c2) && isascii($c3) && isascii($c4)) {
		return "'" . chr($c1) . chr($c2) . chr($c3) . chr($c4) . "'";
	}
	return sprintf("0x%08x", $id);
}


sub dumphdr($)
{
	my ($filename) = @_;
	my $fh;
	my $hdr;
	my $cnt;
	
	open ($fh, "<", $filename) || die "Can't open $filename: $!";
	binmode $fh;
	$cnt = read($fh, $hdr, 512);
	close($fh);
	die "read error on $filename" unless($cnt == 512);
	my ($magic, $flags, $id, $version, $text, $icon, $i_color, $title, $t_color) = unpack("nnNnZ14a[96]nZ18n", $hdr);
	die "invalid magic in $filename" unless ($magic == 100);
	my @flagstr = ();
	push @flagstr, "setonly" if ($flags & 1);
	push @flagstr, "bootinit" if ($flags & 2);
	push @flagstr, "resident" if ($flags & 4);
	printf "Flags: %s\n", join(",", @flagstr);
	printf "Id: %s\n", idstr($id);
	printf "Version: 0x%04x\n", $version;
	printf "IconText: '%s'\n", $text;
	printf "Icon: %*v02x\n", ' ', $_ for unpack("C0a*", $icon);
	printf "IconColor: 0x%04x\n", $i_color;
	printf "Title: '%s'\n", $title;
	printf "TitleColor: 0x%04x\n", $t_color;
}

sub usage()
{
	print "Usage: $me [files]\n";
}

#
# main()
#
my $sawfiles = 0;
while ($#ARGV >= 0) {
	if ($ARGV[0] eq '--help') {
		usage();
		exit(0);
	} elsif ($ARGV[0] eq '--version') {
		print "$me version 1.0\n";
		exit(0);
	} elsif ($ARGV[0] =~ /^-/) {
		die "$me: unknown option $ARGV[0]"
	} else {
		my $filename = $ARGV[0];
		shift @ARGV;
		printf "$filename:\n" if ($sawfiles || $#ARGV >= 0);
		dumphdr($filename);
		$sawfiles = 1;
		printf "\n" unless ($#ARGV < 0);
	}
}	
if (!$sawfiles) {
	print STDERR "$me: missing arguments\n";
	exit(1);
}

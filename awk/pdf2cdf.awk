BEGIN{
	i=0;
}
{
	i=i+$2;
	print $1 "\t" i;
}

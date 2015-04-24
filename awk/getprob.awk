BEGIN{
	now=0;
}
{
	if($1=="]") {
		now=0;
		} 
	if(now==1) {
		print $1 "\t" $2 ;
	} 
	if ($1=="Probabilita"){
		now=1;
	}
}

#ident	"@(#)lp.admin:printers/filters/Text.filt.ls	1.5.3.1"
#ident  "$Header: Text.filt.ls 2.0 91/07/12 $"

#	Module Name: Text.filter.ls

title="Filter Information" 

lifetime=shortterm

`set -l list="/tmp/lp.f3$VPID";`
`set -l flist="/tmp/lp.fl$VPID";`
`echo "$ARG1" | tr -s ',' ' ' | set -l flist;`

done=`/usr/bin/rm  -f $list;
	/usr/bin/rm -f $flist;
	unset -l list -l flist;`

begrow=36
begrow=1
rows=10
scroll=true
columns=72
       `shell "
          for f in $flist
          do
  	echo "Filter: " \$f >> $list;
	$TFADMIN lpfilter -l -f\$f >> $list;
  	echo " "  >> $list;
        done 
	" > $error;`
text="`readfile \$list`"
       


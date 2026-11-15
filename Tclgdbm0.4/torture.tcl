#!/bin/sh
# next line is a comment in tcl \
exec tclsh "$0" ${1+"$@"}

package require Tclgdbm

#
# open
#

proc open {name} {
    return [gdbm open $name rwc]
}


#
# create 1000 entries
#

proc create {db} {

    for {set i 0} {$i < 1000} {incr i} {
	gdbm store $db $i "This data for $i"
    }
}


#
# read all entries
#

proc read1 {db} {

    set key [gdbm firstkey $db]
    set i 0

    while {$key != ""} {
	set data [gdbm fetch $db $key]
	incr i
	set key [gdbm nextkey $db $key]
	
    }
}


#
# read all entries using gdbm list
#

proc read2 {db} {

    set keys [gdbm list $db]
    set i 0

    foreach key $keys {
	set data [gdbm fetch $db $key]
	# puts stdout "$i $key - $data"
	incr i
    }
}

#
# delete 10 percent of all entries
#

proc delete {db} {
    for {set i 0} {$i < 1000} {incr i 3} {
	gdbm delete $db $i
    }
}

#
# lookup all keys
#

proc lookup {db} {

    for {set i 0} {$i < 1000} {incr i} {
	set exists [gdbm exists $db $i]
    }
}


#
# close
#

proc close {db} {
	gdbm close $db
}

##
## main
##

puts "open: \t\t[time {set db [open torture.gdbm]}]"
puts "create: \t[time {create $db}]"
puts "read1: \t\t[time {read1 $db}]"
puts "read2: \t\t[time {read2 $db}]"
puts "delete: \t[time {delete $db}]"
puts "lookup: \t[time {lookup $db}]"
puts "close: \t\t[time {close $db}]"

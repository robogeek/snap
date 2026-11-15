#!/bin/sh
# next line is a comment in tcl \
exec tclsh "$0" ${1+"$@"}

package require Tclgdbm

set db [gdbm open test.data rwc]
foreach i {1 2 3 4 5 6} {
   gdbm store $db $i "This data for $i: [exec /usr/games/fortune -s]"
}

foreach key [lsort [gdbm list $db]] {
   puts stdout "$key [gdbm fetch $db $key]"
}

set key [gdbm firstkey $db]

while {$key != ""} {
  set data [gdbm fetch $db $key]
  puts stdout "found $key: $data"
  set key [gdbm nextkey $db $key]
}

foreach i {2 4 6} {
   gdbm delete $db $i
   puts stdout "$i deleted"
}

foreach i {1 2 3 4 5 6} {
   puts stdout "exists $i == [gdbm exists $db $i]"
}

set key [gdbm firstkey $db]
while {$key != ""} {
  set data [gdbm fetch $db $key]
  puts stdout "found $key: $data"
  set key [gdbm nextkey $db $key]
}

gdbm close $db

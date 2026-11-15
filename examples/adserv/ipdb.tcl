# ipdb.tcl - IP address database

package require dbw 1.0
package provide ipdb 1.0

namespace eval ipdb {

    variable dbhandle
    set dbhandle ""

    variable dbfile
    set dbfile "/path/to/file"

    variable max_showings_remembered
    set max_showings_remembered 10

}


proc ipdb::init {} {

    variable dbhandle
    variable dbfile
    variable adLastIdentifier

    if {$dbhandle != ""} {
	set dbhandle [dbw::new $dbfile "rwc"]
    }
}

proc ipdb::advertShown {ip adID} {
    variable dbhandle
    variable max_showings_remembered

    if [dbw::exists $dbhandle $ip] {
	set info [dbw::get $dbhandle $ip]
    } else {
	set info ""
    }

    set nShowings [llength $info]

    if {$nShowings > $max_showings_remembered} {
	set info [lrange $info 1 end]
    }

    lappend info $adID

    dbw::store $dbhandle $ip $info

    return ""
}

proc ipdb::wasAdShown {ip adID} {
    variable dbhandle
    if [dbw::exists $dbhandle $ip] {
	set info [dbw::get $dbhandle $ip]
	if {[lsearch -exact $info $adID] >= 0} {
	    return 1
	}
    }
    return 0
}


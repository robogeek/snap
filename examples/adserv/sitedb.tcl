# -*- tcl -*-


package require dbw    1.0
package require keydb  1.0
package provide sitedb 1.0
package provide catdb  1.0

namespace eval sitedb {

    variable dbhandle
    set dbhandle ""

    variable dbfile
    set dbfile "/path/to/file"

    variable adLastIdentifier
    set adLastIdentifier ""

    variable adKeyLastIdentifier
    set adKeyLastIdentifier "__AD_LAST_IDENTIFIER__"

    # XXX Need proper schema
    variable schema
    set schema { cat-list }
}

proc sitedb::init {} {

    variable dbhandle
    variable dbfile
    variable adLastIdentifier
    variable schema

    if {$dbhandle != ""} {
	set dbhandle [keydb::new $dbfile "rwc" $schema]
	if ![keydb::exists $dbhandle $adKeyLastIdentifier] {
	    keydb::store $dbhandle $adKeyLastIdentifier "0"
	}
	set adLastIdentifier [keydb::fetch $dbhandle $adKeyLastIdentifier]
    }
}

proc sitedb::fetch  {key} { variable dbhandle; return [keydb::fetch  $dbhandle $key] }
proc sitedb::exists {key} { variable dbhandle; return [keydb::exists $dbhandle $key] }

proc sitedb::findAdvert {siteID ip} {

    variable dbhandle

    if ![exists $siteID] {
	return 0
    }

    set catlist [keydb::getField $dbhandle $siteID cat-list]

    foreach cat $catlist {
	set catID [lindex $cat 0]
	set nShow [lindex $cat 1]
	set shown [lindex $cat 2]

	if {$shown < $nShow} {
	    set adID [catdb::findAd $catID $ip]
	    if {$adID != ""} {
		return [list $siteID $catID $adID $ip]
	    }
	}
    }

    return ""
}

proc sitedb::advertShown {siteID catID adID ip} {

    variable dbhandle
    if ![exists $siteID] {
	return 0
    }

    set catlist [keydb::getField $dbhandle $siteID cat-list]

    set newCatList ""
    set tail ""
    foreach cat $catlist {
	set category [lindex $cat 0]
	set nShow    [lindex $cat 1]
	set shown    [lindex $cat 2]

	if {$catID != $category} {
	    lappend newCatList $cat
	    continue
	}

	incr shown
	if {$nShow == $shown} {
	    set shown 0
	    lappend tail [list $category $nShow $shown]
	} else {
	    lappend newCatList [list $category $nShow $shown]
	}
    }
    foreach t $tail {
	lappend newCatList $t
    }

    keydb::setField $dbhandle $siteID [list cat-list $newCatList ]

    catdb::advertShown $catID $adID $ip
}



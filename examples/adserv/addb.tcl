# addb.tcl - Class for ad database
# -*- tcl -*-

#
# Uses dbw database
#
# Key to dbw is the ad ID #
#
# The key "__AD_LAST_IDENTIFIER__" records the last id# assigned.
#
# Fields in record are
#
#	Ad owner first name
#		 last name
#		 Mail addr line 1
#		 Mail addr line 2
#		 Mail addr line 3
#		 Mail addr line 4
#		 Mail addr line 5
#		 Mail addr line 6
#		 EMail addr
#		 Web site URL
#	Category list, each containing:
#		Category key
#		Number of shows (out of 100)
#		Number times shown
#	GIF file name
#

package require dbw 1.0
package provide addb 1.0

namespace eval addb {

    variable dbhandle
    set dbhandle ""

    variable dbfile
    set dbfile "/path/to/file"

    variable adLastIdentifier
    set adLastIdentifier ""

    variable adKeyLastIdentifier
    set adKeyLastIdentifier "__AD_LAST_IDENTIFIER__"

    variable schema
    set schema {
	owner-first-name
	owner-last-name
	owner-mail-line-1
	owner-mail-line-2
	owner-mail-line-3
	owner-mail-line-4
	owner-mail-line-5
	owner-mail-line-6
	owner-email-addr
	owner-site-url
	category-list
	gif-file-name
    }
}

proc addb::init {} {

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

proc addb::fetch  {key} { variable dbhandle; return [keydb::fetch  $dbhandle $key] }
proc addb::exists {key} { variable dbhandle; return [keydb::exists $dbhandle $key] }

proc addb::getCategories {key} {
    variable dbhandle
    return [keydb::getField $dbhandle $key category-list]
}

proc addb::getCategoryInfo {key catID} {
    variable dbhandle
    set catList [getCategories $key]

    foreach cat $catList {
	set id [lindex $cat 0]
	if {$id == $catID} {
	    return $cat
	}
    }
    return ""
}

proc addb::canDisplayAd {adID catID ip} {
    variable dbhandle

    if ![exists $adID]              { return 0 }
    if [ipdb::wasAdShown $ip $adID] { return 0 }

    set catlist [keydb::getField $dbhandle $key category-list]

    foreach cat $catlist {
	set id [lindex $cat 0]
	if {$catID != $id} {
	    continue
	}

	set nShow [lindex $cat 1]
	set shown [lindex $cat 2]

	if {$shown < $nShow} {
	    return 1
	}
    }

    return 0
}

proc addb::showAdvert {info} {
    variable dbhandle

    set siteID [lindex $info 0]
    set catID  [lindex $info 1]
    set adID   [lindex $info 2]
    set ip     [lindex $info 3]

    # XXX Stream out the GIF

    sitedb::advertShown $siteID $catID $adID $ip
}

proc addb::advertShown {adID catID ip} {

    variable dbhandle
    if ![exists $adID] {
	return 0
    }

    set catlist [keydb::getField $dbhandle $adID category-list]

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

    keydb::setField $dbhandle $adID [list category-list $newCatList ]

    ipdb::adShown $ip $adID
}


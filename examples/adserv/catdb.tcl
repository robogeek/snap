
package require dbw 1.0
package provide catdb 1.0

namespace eval catdb {

    variable dbhandle
    set dbhandle ""

    variable dbfile
    set dbfile "/path/to/file"

    # XXX Need proper schema
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
	ad-list
    }

}

proc catdb::init {} {

    variable dbhandle
    variable dbfile
    variable schema

    if {$dbhandle != ""} {
	set dbhandle [keydb::new $dbfile "rwc" $schema]
    }
}

proc catdb::fetch {key} { variable dbhandle; return [keydb::fetch $dbhandle $key] }

proc catdb::findAd {catID ip} {

    variable dbhandle

    set adlist [keydb::getField $dbhandle $catID ad-list]
    foreach adID $adlist {
	if [addb::canDisplayAd $adID $catID $ip] {
	    return $adID
	}
    }

    return ""
}

proc catdb::advertShown {catID adID ip} {

    variable dbhandle

    set adlist [keydb::getField $dbhandle $catID ad-list]
    if {[lsearch -exact $adlist $adID] >= 0} {
	return [addb::advertShown $adID $catID $ip]
    }

    error "Advert with key $adID not present in category $catID"
}

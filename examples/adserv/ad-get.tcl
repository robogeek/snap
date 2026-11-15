
# -*- tcl -*- 

package require addb   1.0
package require catdb  1.0
package require ipdb   1.0
package require sitedb 1.0

set ip [cgi get XXX] ;# XXX Need to find out how to get the IP addr
set id [cgi get id]

addb::init
catdb::init
ipdb::init
sitedb::init

addb::showAdvert [sitedb::findAdvert $id $ip]

exit 0

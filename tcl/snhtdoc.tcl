# snhtdoc.tcl - HTML wrapper classes for SNAP -*- tcl -*-

package provide snhtdoc 1.0

namespace eval snhtdoc {


    namespace export href goober

}

snap_template snhtdoc::href {url text {anchor ""}} {
<A href="<{ if {$anchor != ""} { return "$url#$anchor"} else { return $url } }>">
    <{ return $text }>
</a>
}

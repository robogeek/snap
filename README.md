snap - Making web sites a snap! ;-)

The code in this repository was written in the mid-90s and does not represent any kind of modern best practice.

At that time I wanted server-side dynamic processing that would produce HTML pages.  I didn't want to use Perl, nor PHP, which were the prime examples at the time.  I also was highly enthusiastic about TCL, and therefore based this on TCL.

Hence, this is a system built on TCL and able to be installed in a web-hosting environment that allowed CGI scripts.  This is old-school web hosting.

The core is a templating system allowing us to write, in the middle of TCL code, 

```
snap_template snhtdoc::href {url text {anchor ""}} {
                <A href="<$url><{ if {$anchor != ""} {
                                return "$url#$anchor"
                        } }>"><$text></a>
                }
```

The argument to `snap_template` starting with `{` is HTML, with a twist.  `<$url>` references the variable `url` while `<{ ... }>` is a way to embed TCL code.


That's about all I remember.  I don't see any documentation.  Well, I did write some documentation as comments in the code.

Here's a teensy bit more from the etc directory:

```
#!snap -snap_subst
<HTML>
<HEAD>
<TITLE>This is a test</TITLE>
</HEAD>
<BODY>
<TABLE>
<TR><TD><{ snap_include "navbar" }><TD><{ snap_include "body" }></TR>
</TABLE>
<P>
<p align="<{ return center }>">
</BODY>
```

Clearly this would be installed as an executable in the filesystem.  The `#!` hashbang says that the binary, `snap`, would be in the PATH.  In modern times the hash-bang line would be a little different.

The `-snap_subst` argument would mean to interpret the remaining stuff using the template system.

The command `<{ snap_include ... }>` clearly looks for and processes the named file, inserting its output into the HTML.

This is another example:

```
#
# calHtml - Using the cal(1) program, create an HTML table
#       showing the matching calendar month.
#


snap template calHtml {{month ""} {year ""}} {
<{
    set f [open "|cal $month $year" r]
    set lines [split [read $f] "\n"]
    close $f
    return ""
}><table>
<caption><b><{ lindex $lines 0 }></b></caption>
<tr><th>S</th><th>M</th><th>Tu</th><th>W</th><th>Th</th><th>F</th><th>S</th><th></tr>
<{
    set ret ""
    foreach l [lrange $lines 2 end] {
        if {[llength $l] <= 0} { continue }
        append ret "<tr>"
        while {[regexp {^(...)(.*)$} $l match head rest] != 0} {
            append ret "<td>$head</td>"
            set l $rest
        }
        if {$l != ""} { append ret "<td>$l</td>" }
        append ret "</tr>\n"
    }
    return $ret
}></table>
}
```



A more complete example is - examples/adserv - which is an attempt to serve "advertisements" onto a webpage.


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

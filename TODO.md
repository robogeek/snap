TODO list for snap as of May 12, 1998

- Implement memory channel (memchan) use as so

	(Auto)-detect or otherwise know when we are to put
	out a header.

	Two channels - 1) header, 2) html
	[MOOT???  Have 'snap header' command now to add to header
	 and it does not use memchan.]

	Start header memchan with "Content-Type: text/html".

	snap_subst, snap_include, and snap_template do their
	output to an html channel.

	Both channels configured to translate output lines
	to have CRLF because that's the HTTP spec.

	When it is done processing the C code will then
	access both channels to create the output.

	This means that 1) included TCL code need not ensure
	their return value is appropriate, 2) need not accumulate
	the HTML output and make it the return value.

	Q: Is there an instance where you want to snap_subst
	(or other snap_ command) and not automatically go
	to the html channel?

	A: Hmm...  Consider

		snap_template snhtdoc::href {url text {anchor ""}} {
		<A href="<$url><{ if {$anchor != ""} {
				return "$url#$anchor"
			} }>"><$text></a>
		}

	Solution: In snap_subst, as it is treading through
	the input it immediately puts it to the memchan as
	it is going.  In fact we can follow the existing pattern
	now of attaching it to the result - just have it sent
	to the memchan.

		snap_template snhtdoc::href {url text {anchor ""}} {
		<A href="<$url><{ if {$anchor != ""} {
				snap_subst {#<$anchor>}
			} }>"><$text></a>
		}


- Rewrite dbw && hierdb as a C module.  Speed.  Hiding it in C.

- YACC based parser.  Parse XML.  Translates the XML to HTML
  if needed.

- Test suites: snap, dbw (finish), hierdb, memchan.

- Suite of common HTML templates in snap_template form.
  See snhtdoc.tcl for beginnings.

- App for building templates in semi-wysiwyg form:

	Source window for main template.  Syntax colored.

	Result window or way of using IE and/or Navigator to show
	the result.  (Result is - run the template, capture the
	output HTML & header, send HTML to browser for display)

	Editing other source files.

	CGI argument and environment variable setting area.
	This lets the template be run in a pseudo-CGI mode.

	Area to type in a command and view results.  (Console?)

- Incorporate gd and free-type support.

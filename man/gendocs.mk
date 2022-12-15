
gen-docs: crosswrench.mdoc .PHONY
	mandoc -T html ${.ALLSRC} > crosswrench.1.html
	mandoc -T man ${.ALLSRC} > crosswrench.1

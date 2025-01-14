(TeX-add-style-hook
 "chronal-calendar"
 (lambda ()
   (TeX-add-to-alist 'LaTeX-provided-class-options
                     '(("article" "12pt")))
   (TeX-run-style-hooks
    "latex2e"
    "article"
    "art12"
    "amssymb"
    "amsmath"
    "amsthm"
    "fullpage"
    "enumitem"
    "multicol"
    "longtable"
    "verbatim"
    "anyfontsize"
    "diagbox"
    "tikz")
   (LaTeX-add-xcolor-definecolors
    "hexcolor0xf81e1c"
    "hexcolor0x3c00ff"))
 :latex)


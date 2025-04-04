# create convenience target that bundles all reg perfflowaspect deps
add_library(perfflowaspect::perfflowaspect INTERFACE IMPORTED)

set_property(TARGET perfflowaspect::perfflowaspect
             PROPERTY INTERFACE_LINK_LIBRARIES
             perfflowaspect)

add_doc( devhandbook libyzis nyzis qyzis )

set( index_path devhandbook/html )
configure_file( index.html.in ${apidir}/index.html )
configure_file( mainpage.dx.in ${CMAKE_CURRENT_BINARY_DIR}/mainpage.dx )

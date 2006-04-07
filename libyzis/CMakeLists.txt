#kde4_header()

set(prefix              ${CMAKE_INSTALL_PREFIX})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/translator.h.in ${CMAKE_CURRENT_BINARY_DIR}/translator.h)


#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/translator.h )

include_directories( ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR}/libyzis ${CMAKE_SOURCE_DIR}/libyzis ${YZIS_INCLUDES} )


########### next target ###############

set(yzis_LIB_SRCS 
   action.cpp 
   attribute.cpp 
   buffer.cpp 
   color.cpp 
   cursor.cpp 
   debug.cpp 
   drawbuffer.cpp 
   events.cpp 
   ex_lua.cpp 
   folding.cpp 
   font.cpp 
   history.cpp 
   internal_options.cpp 
   line.cpp 
   linesearch.cpp 
   mapping.cpp 
   mark.cpp 
   mode_command.cpp 
   mode_complete.cpp 
   mode.cpp 
   mode_ex.cpp 
   mode_insert.cpp 
   mode_search.cpp 
   mode_visual.cpp 
   option.cpp 
   readtags.c 
   registers.cpp 
   schema.cpp 
   search.cpp 
   selection.cpp 
   session.cpp 
   swapfile.cpp 
   syntaxdocument.cpp 
   syntaxhighlight.cpp 
   tags_interface.cpp 
   tags_stack.cpp 
   undo.cpp 
   view.cpp 
   viewcursor.cpp 
   viewid.cpp 
   yzisinfo.cpp 
   yzisinfojumplistrecord.cpp 
   yzisinfostartpositionrecord.cpp 
)

#add files needing MOC here :)
set(yzis_LIB_MOCS 
   session.h
)

#kde4_automoc(${yzis_LIB_SRCS})
QT4_WRAP_CPP(yzis_LIB_MOC_SRCS ${yzis_LIB_MOCS} )

#kde4_add_library(yzis SHARED ${yzis_LIB_SRCS} )
add_library(yzis SHARED ${yzis_LIB_SRCS} ${yzis_LIB_MOC_SRCS} )

target_link_libraries(yzis ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${LIBLUA50_LIBRARIES} ${LIBLUALIB50_LIBRARIES} ${MAGIC_LIBRARIES} ${GETTEXT_LIBRARIES})

set_target_properties(yzis PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(/lib yzis )


########### install files ###############

install_files(/share/yzis FILES magic.mime )

#kde4_footer()



project(libyzis)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/translator.h.in ${CMAKE_CURRENT_BINARY_DIR}/translator.h)
#macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/translator.h )

include_directories(${YZIS_INCLUDES})

########### next target ###############

set(yzis_LIB_SRCS 
   action.cpp 
   buffer.cpp 
   color.cpp 
   cursor.cpp 
   debug.cpp 
   drawbuffer.cpp 
   events.cpp 
   luaengine.cpp 
   luaregexp.cpp 
   luafuncs.cpp 
   folding.cpp 
   font.cpp 
   history.cpp 
   internal_options.cpp 
   keys.cpp
   line.cpp 
   linesearch.cpp 
   mapping.cpp 
   mark.cpp 
   mode_pool.cpp 
   mode_command.cpp 
   mode_complete.cpp 
   mode.cpp 
   mode_ex.cpp 
   mode_insert.cpp 
   mode_search.cpp 
   mode_visual.cpp 
   option.cpp 
   registers.cpp 
   resourcemgr.cpp 
   search.cpp 
   selection.cpp 
   session.cpp 
   swapfile.cpp 
   tags_interface.cpp 
   tags_stack.cpp 
   undo.cpp 
   view.cpp 
   viewcursor.cpp 
   yzisinfo.cpp 
   yzisinfojumplistrecord.cpp 
   yzisinfostartpositionrecord.cpp 

   readtags/readtags.c 

   kate/attribute.cpp 
   kate/schema.cpp 
   kate/syntaxdocument.cpp 
   kate/syntaxhighlight.cpp 
)

qt4_wrap_cpp(yzis_LIB_MOC_SRCS ${yzis_LIB_MOCS})

add_library(yzis SHARED ${yzis_LIB_SRCS} ${yzis_LIB_MOC_SRCS} )

message( LIBLUA_LIBRARIES = ${LIBLUA_LIBRARIES} )
message( LIBLUA51_LIBRARIES = ${LIBLUA51_LIBRARIES} )

target_link_libraries(yzis ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${LIBLUA_LIBRARIES} ${MAGIC_LIBRARIES} ${GETTEXT_LIBRARIES})

set_target_properties(yzis PROPERTIES VERSION 0.0.0 SOVERSION 0 )

if (WIN32)
    install(TARGETS yzis 
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION bin
            LIBRARY DESTINATION lib
            )
else (WIN32)
    install(TARGETS yzis 
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            )
endif (WIN32)
install(FILES magic.mime DESTINATION share/yzis)


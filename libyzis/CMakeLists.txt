kde4_header()

set(prefix              ${CMAKE_INSTALL_PREFIX})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/translator.h.in ${CMAKE_CURRENT_BINARY_DIR}/translator.h @ONLY)

find_package(LibLua50 REQUIRED)
find_package(LibLuaLib50 REQUIRED)
find_package(Magic REQUIRED)

macro_additional_clean_files( ${CMAKE_CURRENT_BINARY_DIR}/translator.h )

include_directories( ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/libyzis ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${LIBLUA50_INCLUDE_DIR} ${LIBLUALIB50_INCLUDE_DIR} ${MAGIC_INCLUDE_DIR} )


########### next target ###############

set(yzis_LIB_SRCS 
   session.cpp 
   mode.cpp 
   mode_command.cpp 
   mode_search.cpp 
   mode_visual.cpp 
   mode_ex.cpp 
   mode_insert.cpp 
   view.cpp 
   search.cpp 
   mark.cpp 
   action.cpp 
   cursor.cpp 
   selection.cpp 
   line.cpp 
   buffer.cpp 
   linesearch.cpp 
   debug.cpp 
   internal_options.cpp 
   option.cpp 
   registers.cpp 
   undo.cpp 
   ex_lua.cpp 
   swapfile.cpp 
   schema.cpp 
   viewcursor.cpp 
   mapping.cpp 
   events.cpp 
   syntaxdocument.cpp 
   syntaxhighlight.cpp 
   attribute.cpp 
   yzisinfo.cpp 
   yzisinfostartpositionrecord.cpp 
   yzisinfojumplistrecord.cpp 
   readtags.c 
   tags_interface.cpp 
   tags_stack.cpp 
   mode_complete.cpp 
   viewid.cpp 
   history.cpp 
   color.cpp 
   drawbuffer.cpp 
   folding.cpp 
   font.cpp 
   )

kde4_automoc(${yzis_LIB_SRCS})

kde4_add_library(yzis SHARED ${yzis_LIB_SRCS})

target_link_libraries(yzis  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${LIBLUA50_LIBRARIES} ${LIBLUALIB50_LIBRARIES} ${MAGIC_LIBRARIES} )

set_target_properties(yzis PROPERTIES VERSION 0.0.0 SOVERSION 0 )
install_targets(${LIB_INSTALL_DIR} yzis )


########### install files ###############

install_files(  FILES  magic.mime )

kde4_footer()


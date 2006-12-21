
errorMsg= \
"In order to compile yzis with lua support, you need to set the following variables: LUAINCLUDE to directory containing lua.h and lualib.h, LUALIB to directory containing liblua.lib and liblualib.lib. \ 
You can set them either as environment variables or when running qmake: \
qmake LUAINCLUDE=d:/program/lua/include LUALIB=d:/program/lua/lib \
"

isEmpty( LUAINCLUDE ) { LUAINCLUDE=$$(LUAINCLUDE) }
isEmpty( LUALIB ) { LUALIB=$$(LUALIB) }

isEmpty( LUAINCLUDE ) { error( $$errorMsg ) }
isEmpty( LUALIB )     { error( $$errorMsg ) }


!exists( $$LUAINCLUDE/lua.h ) { error( Could not find lua.h in $$LUAINCLUDE ) }
!exists( $$LUAINCLUDE/lualib.h ) { error( Could not find lualib.h in $$LUAINCLUDE ) }



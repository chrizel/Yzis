--[[

Description: Test insert mode in yzis.

Author: Krzysztof Goj
Version: 0.1
License: LGPL

]]--

require('luaunit')
require('utils')


TestInsert = {} --class
    function TestInsert:setUp() 
        clearBuffer()
    end

    function TestInsert:tearDown() 
        sendkeys( "<ESC>" )
        clearBuffer()
    end

    function TestInsert:test_initial_state()
        assertEquals(bufferContent(),"")
        assertPos(1,1)
        assertEquals(mode(), MODE_NORMAL)

        sendkeys( "i" )
        --printBufferContent()
        assertEquals(mode(), MODE_INSERT)
        assertPos(1,1)
        assertEquals(bufferContent(),"")
    end

    function TestInsert:test_delete()
        sendkeys( "iAla has a cat." )
        assertEquals(mode(), MODE_INSERT)
        assertPos( 1, 15 )
        sendkeys( "<C-h>" )
        assertEquals( bufferContent(), "Ala has a cat" )
        assertPos( 1, 14 )
        sendkeys( "<BS>" )
        assertEquals( bufferContent(), "Ala has a ca" )
        assertPos( 1, 13 )
        sendkeys( "<C-w>" )
        assertEquals( bufferContent(), "Ala has a " )
        assertPos( 1, 11 )
        sendkeys( "<C-[>" )
        assertEquals(mode(), MODE_NORMAL)
        assertPos( 1, 10 )
        sendkeys( "i" )
        assertEquals(mode(), MODE_INSERT)
        assertPos( 1, 10 )
        sendkeys( "<ESC>" )
        assertEquals(mode(), MODE_NORMAL)
        assertPos( 1, 9 )
    end
	

if not _REQUIREDNAME then
    -- ret = LuaUnit:run('TestInsert:test_delete') -- will execute only one test
    ret = LuaUnit:run() -- will execute all tests
    setLuaReturnValue( ret )
end


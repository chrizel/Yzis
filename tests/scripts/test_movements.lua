
require('luaunit')
require('utils')

function assertPos(line,col)
    assertEquals( line, winline() )
    assertEquals( col, wincol() )
end

TestMovements = {} --class
    function TestMovements:setUp() 
        clearBuffer()
    end

    function TestMovements:tearDown() 
        clearBuffer()
    end

    function TestMovements:test_initial_state()
        assertEquals(bufferContent(),"")
        assertPos(1,1)
    end


if not _REQUIREDNAME then
    -- luaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
    -- luaUnit:run('TestLuaBinding') -- will execute only one class of test
    luaUnit:run() -- will execute all tests
end


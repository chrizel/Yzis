

require('luaunit')
require('utils')

TestExCommands = {} --class
    function TestMovements:setUp() 
        clearBuffer()
    end

    function TestMovements:tearDown() 
        clearBuffer()
    end

    function TestMovements:test_initial_state()
        assertEquals(bufferContent(),"")
    end


if not _REQUIREDNAME then
    -- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
    -- LuaUnit:run('TestLuaBinding') -- will execute only one class of test
    LuaUnit:run() -- will execute all tests
end


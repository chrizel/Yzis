

require('luaunit')
require('utils')

TestExCommands = {} --class
    function TestExCommands:setUp() 
        clearBuffer()
    end

    function TestExCommands:tearDown() 
        clearBuffer()
    end

    function TestExCommands:test_initial_state()
        assertEquals(bufferContent(),"")
    end


if not _REQUIREDNAME then
    -- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
    -- LuaUnit:run('TestLuaBinding') -- will execute only one class of test
    LuaUnit:run() -- will execute all tests
end


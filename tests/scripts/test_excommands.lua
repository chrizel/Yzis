

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
    -- ret = LuaUnit:run('TestExCommands:test_initial_state') -- will execute only one test
    ret = LuaUnit:run() -- will execute all tests
    setLuaReturnValue( ret )
end


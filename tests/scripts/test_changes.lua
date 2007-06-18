--[[
Description: Test command mode commands that change text.
]]--

require('luaunit')
require('utils')


TestChanges = {} --class
    function TestChanges:setUp() 
        clearBuffer()
    end

    function TestChanges:tearDown() 
        clearBuffer()
    end

    function TestChanges:test_r()
	assertEquals(bufferContent(), "")
	assertPos(1,1)
	insertline(1, "WORD")
	assertPos(1,4)
	sendkeys("rd")
	assertPos(1,4)
	assertEquals(bufferContent(), "WORd")
	sendkeys("0")
	assertPos(1,1)
	sendkeys("r")
	sendkeys("<ESC>")
	assertPos(1,1)
	assertEquals(bufferContent(), "WORd")
	sendkeys("rw")
	assertPos(1,1)
	assertEquals(bufferContent(), "wORd")
	clearBuffer()
    end

if not _REQUIREDNAME then
    -- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
    -- LuaUnit:run('TestLuaBinding') -- will execute only one class of test
    LuaUnit:run() -- will execute all tests
end



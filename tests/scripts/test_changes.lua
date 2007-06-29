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

    function TestChanges:test_d()
	assertEquals(bufferContent(), "")
	assertPos(1,1)
	insertline(1, "WORD")
	assertPos(1,4)
	goto(1,1)
	sendkeys("d$")
	assertEquals(bufferContent(), "")
	assertPos(1,1)
	insertline(1, "WORD")
	assertPos(1,4)
	sendkeys("dd")
	assertEquals(bufferContent(), "")
	assertPos(1,1)
	insertline(1, "LINE 1")
	assertPos(1,6)
	insertline(2, "LINE 2")
	assertPos(2,6)
	sendkeys("dd")
	assertEquals(bufferContent(), "LINE 1")
	assertPos(1,1)
	sendkeys("dd")
	assertEquals(bufferContent(), "")

	assertPos(1,1)
	insertline(1, "WORD")
	goto(1,2)
	sendkeys("D")
	assertEquals(bufferContent(), "W")
	assertPos(1,1)
	sendkeys("D")
	assertEquals(bufferContent(), "")

	assertPos(1,1)
	insertline(1, "LINE 1")
	assertPos(1,6)
	insertline(2, "LINE 2")
	assertPos(2,6)
	insertline(3, "LINE 3")
	assertPos(3,6)
	goto(1,1)
	sendkeys("dG")
	assertEquals(bufferContent(), "")
    end

if not _REQUIREDNAME then
    -- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
    -- LuaUnit:run('TestLuaBinding') -- will execute only one class of test
    LuaUnit:run() -- will execute all tests
end



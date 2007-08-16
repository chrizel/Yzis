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
	goto(1,1)
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
	goto(2,1)
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

	assertPos(1,1)
	insertline(1, "LINE 1")
	insertline(2, "LINE 2")
	goto(1,1)
	sendkeys("dj")
	assertEquals(bufferContent(), "")
    end

    function TestChanges:test_ctrl_a()
	assertEquals(bufferContent(), "")
	assertPos(1,1)
	insertline(1, "A 0 B")
	goto(1,1)
	sendkeys("<CTRL>a")
	assertEquals(bufferContent(), "A 0 B")
	assertPos(1,1)

	goto(3,1)
	sendkeys("<CTRL>a")
	assertEquals(bufferContent(), "A 1 B")
	assertPos(1,3)
	sendkeys("30<CTRL>a")
	assertEquals(bufferContent(), "A 31 B")
	assertPos(1, 4)

	clearBuffer()

	insertline(1, "B -5 A")
	goto(4,1)
	sendkeys("<CTRL>a")
	assertEquals(bufferContent(), "B -4 A")
	assertPos(1,4)

	goto(3,1)
	sendkeys("4<CTRL>a")
	assertEquals(bufferContent(), "B 0 A")
	assertPos(1,3)
	clearBuffer()
    end

    function TestChanges:test_ctrl_x()
	assertEquals(bufferContent(), "")
	assertPos(1,1)
	insertline(1, "A 0 B")
	goto(1,1)
	sendkeys("<CTRL>x")
	assertEquals(bufferContent(), "A 0 B")
	assertPos(1,1)

	goto(3,1)
	sendkeys("<CTRL>x")
	assertEquals(bufferContent(), "A -1 B")
	assertPos(1,4)
	sendkeys("30<CTRL>x")
	assertEquals(bufferContent(), "A -31 B")
	assertPos(1,5)

	clearBuffer()

	insertline(1, "B 5 A")
	goto(3,1)
	sendkeys("<CTRL>x")
	assertEquals(bufferContent(), "B 4 A")
	assertPos(1,3)

	goto(3,1)
	sendkeys("4<CTRL>x")
	assertEquals(bufferContent(), "B 0 A")
	assertPos(1,3)
	clearBuffer()
    end

if not _REQUIREDNAME then
    -- ret = LuaUnit:run('TestChanges:test_ctrl_x') -- will execute only one test
    ret = LuaUnit:run() -- will execute all tests
    setLuaReturnValue( ret )
end



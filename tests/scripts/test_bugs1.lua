require('luaunit')
require('utils')

TestBugs1 = {} --class
    function TestBugs1:setUp() 
        clearBuffer()
    end

    function TestBugs1:tearDown() 
        clearBuffer()
    end

	function TestBugs1:test_bug141()
		clearBuffer()
		sendkeys("i	void myfunction() {<ENTER>	int doingsomething;<ESC>")
		assertEquals(bufferContent(), "	void myfunction() {\n	int doingsomething;")
		sendkeys("ggA")
		assertPos(1, 21)
		sendkeys("<DEL>")
		assertEquals(bufferContent(), "	void myfunction() { int doingsomething;")
	end

if not _REQUIREDNAME then
    -- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
    -- LuaUnit:run('TestLuaBinding') -- will execute only one class of test
    LuaUnit:run() -- will execute all tests
end

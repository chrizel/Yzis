
require('luaunit')
require('utils')

TestGoto = {}
	function TestGoto:setUp()
		clearBuffer()
	end

	function TestGoto:tearDown()
		clearBuffer()
	end

	function TestGoto:test_stickyCol()
		clearBuffer()
		insertline(1, "line1 line1 line1 line1 line1")
		insertline(2, "		line2 line2 line2 line2")
		goto(1,1)
		sendkeys("2lj")
		assertScrPos(2,1)
		assertPos(2,1)
		sendkeys("k")
		assertScrPos(1,3)
		assertPos(1,3)
		sendkeys("8lj")
		assertScrPos(2,9)
		assertPos(2,2)
		sendkeys("k")
		assertScrPos(1,11)
		assertPos(1,11)
		sendkeys("8l")
		assertPos(1,19)
		sendkeys("j")
		assertScrPos(2,19)
		assertPos(2,5)
		sendkeys("k")
		assertScrPos(1,19)
		sendkeys("j$")
		assertPos(2,25)
		assertScrPos(2,39)
		sendkeys("k")
		assertPos(1,29)
		assertScrPos(1,29)
	end

if not _REQUIREDNAME then
	LuaUnit:run() -- will execute all tests
end


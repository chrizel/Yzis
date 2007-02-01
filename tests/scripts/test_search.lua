--[[

Description: Test the search feature of yzis

Author: Philippe Fremy
Version: 0.1
License: LGPL

]]--
require('luaunit')
require('utils')

TestSearch = {} --class
    function TestSearch:setUp() 
        clearBuffer()
    end

    function TestSearch:tearDown() 
        clearBuffer()
    end

	function TestSearch:test_search_case_sensitive()
		clearBuffer()
		sendkeys("itest TeSt TEST test1 1test 1Test<ENTER> test <ENTER> TEST <ENTER> test1 <ESC>")
		goto(1,1)
		assertPos(1, 1)
		sendkeys("/test<ENTER>")
		assertPos(1, 16)
		sendkeys("n")
		assertPos(1, 23)
		sendkeys("n")
		assertPos(2, 2)
		sendkeys("n")
		assertPos(4, 2)
		goto(1,1)
		assertPos(1, 1)
		sendkeys("/TEST<ENTER>")
		assertPos(1, 11)
		sendkeys("<ESC>")
	end

	function TestSearch:test_search_case_insensitive()
		clearBuffer()
		sendkeys("itest TeSt TEST test1 1test 1Test<ENTER> test <ENTER> TEST <ENTER> test1 <ESC>")
		goto(1,1)
		assertPos(1, 1)
		sendkeys("/test\\c<ENTER>")
		assertPos(1, 6)
		sendkeys("n")
		assertPos(1, 11)
		sendkeys("n")
		assertPos(1, 16)
		sendkeys("n")
		assertPos(1, 23)
		sendkeys("n")
		assertPos(1, 29)
		sendkeys("n")
		assertPos(2, 2)
	end

if not _REQUIREDNAME then
     LuaUnit:run('TestSearch:test_search_case_sensitive') -- will execute only one test
    -- LuaUnit:run('TestLuaBinding') -- will execute only one class of test
    --LuaUnit:run() -- will execute all tests
end


require('luaunit')
require('utils')

Test_LHM_commands = {} --class
    function Test_LHM_commands:setUp() 
        clearBuffer()
    end

    function Test_LHM_commands:tearDown() 
        clearBuffer()
    end

 
    function Test_LHM_commands:test_H()
	--[[ Test the H command ]]--
        assertEquals( bufferContent(), "" )
	sendkeys("itest TeSt TEST test1 1test 1Test<ESC>")
	sendkeys("otest again<ESC>")
	sendkeys("o    TEST <ESC>")
	sendkeys("otest1 test<ESC>")
	sendkeys("o<ESC>")
	sendkeys("o       7 spaces begining...<ESC>")
	sendkeys("otest1 test<ESC>")
	sendkeys("GH")

	assertPos(1,1) 
	sendkeys("Gll3H")
	assertPos(3,5)
	sendkeys("2j5H")
	assertPos(5,1)
	sendkeys("6H")
	assertPos(6,8)
    end

    function Test_LHM_commands:test_L()
    	--[[ Test the L command ]]--
        assertEquals( bufferContent(), "" )
	sendkeys("itest TeSt TEST test1 1test 1Test<ESC>")
	sendkeys("otest again<ESC>")
	sendkeys("o    TEST <ESC>")
	sendkeys("otest1 test<ESC>")
	sendkeys("o<ESC>")
	sendkeys("o       7 spaces begining...<ESC>")
	sendkeys("otest1 test<ESC>")
	sendkeys("GL")

	assertPos(7,1) 
	sendkeys("ggll3L")
	assertPos(5,1)
	sendkeys("gg2j4L")
	assertPos(4,1)
	sendkeys("2L")
	assertPos(6,8)
    end
    function Test_LHM_commands:test_M()
        assertEquals( bufferContent(), "" )
	--[[ pair number of lines ]]--
	sendkeys("itest<ESC>")
	sendkeys("o    test<ESC>")
	sendkeys("otest<ESC>")
	sendkeys("o    test<ESC>")
	sendkeys("M")
	assertPos(2,5)

	--[[ odd number of lines ]]--
	sendkeys("Gotest<ESC>")
	sendkeys("M")
	assertPos(3,1)
   end


if not _REQUIREDNAME then
    -- ret = LuaUnit:run('Test_LHM_commands:test_char_movement') -- will execute only one test
    ret = LuaUnit:run() -- will execute all tests
    setLuaReturnValue( ret )
end


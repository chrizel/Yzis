
-- Unit testing starts
require('luaunit')

TestLuaBinding = {} --class
    function TestLuaBinding:setUp() 
        -- this function is run before each test, so that multiple
        -- tests can share initialisations
    end

    function TestLuaBinding:tearDown() 
        -- this function is executed after each test
        -- here, we have nothing to do so we could have avoid
        -- declaring it
    end

    function TestLuaBinding:Xtest_use_all_functions()
		require('use_all_functions')
    end

    function TestLuaBinding:test_version()
        -- ok, simple one
        s = version()
        assertEquals( string.len(s) > 0, true )
    end

luaUnit:run()


-- Unit testing starts
require('luaunit')

TestLuaBinding = {} --class
    function TestLuaBinding:setUp() 
        -- this function is run before each test, so that multiple
        -- tests can share initialisations
        self.a = 1
        self.b = 2
    end

    function TestLuaBinding:tearDown() 
        -- this function is executed after each test
        -- here, we have nothing to do so we could have avoid
        -- declaring it
    end

    function TestLuaBinding:test_use_all_functions()
		require('use_all_functions')
    end

luaUnit:run()

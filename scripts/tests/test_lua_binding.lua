
-- Unit testing starts
require('luaunit')

function bufferContent()
    local i,s
    i = 2
    s = line(1)
    while i<=linecount() do
        s = s.."\n".. line(i)
        i = i + 1
    end
    return s
end

function printBufferContent()
    local i=1
    print("Buffer content:")
    while i<=linecount() do
        print("line "..i.." : '"..line(i).."'")
        i = i + 1
    end
end

TestLuaBinding = {} --class
    function TestLuaBinding:setUp() 
        while linecount() > 1 do
            delete(1)
        end
        delete(1)
    end

    function TestLuaBinding:tearDown() 
    end

    function TestLuaBinding:test_setup()
        -- check that setup correctly clears the buffer
        insert(1,1,"coucou")
        assertEquals(line(1), "coucou" )
        self.setUp()
        assertEquals(line(1), "" )
        assertEquals( bufferContent(), "" )
    end

    function TestLuaBinding:Xtest_use_all_functions() -- disabled for now
		require('use_all_functions')
    end

    function TestLuaBinding:test_version()
        -- ok, simple one
        s = version()
        assertEquals( string.len(s) > 0, true )
    end

    function TestLuaBinding:test_linecount()
        assertEquals( 1, linecount() )
        insert(1,1,"coucou")
        -- still only one line
        assertEquals( 1, linecount() )
    end

    function TestLuaBinding:test_insert()
        local s = "coucou"
        insert(1,1,s)
        assertEquals( line(1), s )
        insert(1,1,"a")
        assertEquals( line(1), 'a'..s )
        insert(8,1,"z")
        assertEquals( line(1), 'a'..s..'z' )

        -- insert on a missing line
        insert(1,2,s)
        assertEquals( line(1), 'a'..s..'z' )
        assertEquals( line(2), s )
        content = bufferContent()

        -- insert beyond the end of file does nothing
        insert(1,4,s)
        printBufferContent()
        assertEquals( bufferContent(), content )
    end

    function TestLuaBinding:test_insert_multiline()
        assertEquals( bufferContent(), "" )
        local s = "coucou\nhop"
        insert(1,1,s)
        printBufferContent()
        assertEquals( bufferContent(), s )
    end



luaUnit:run()

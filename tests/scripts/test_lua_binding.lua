
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
    function TestLuaBinding:clearBuffer()
        while linecount() > 1 do
            deleteline(1)
        end
        deleteline(1)
    end

    function TestLuaBinding:setUp() 
        self:clearBuffer()
    end

    function TestLuaBinding:tearDown() 
        self:clearBuffer()
    end

    function TestLuaBinding:test_setup()
        -- check that setup correctly clears the buffer
        insert(1,1,"coucou")
        assertEquals(line(1), "coucou" )
        self:setUp()
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
        appendline("hop")
        assertEquals( 2, linecount() )
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
        assertEquals( bufferContent(), content )
    end

    function TestLuaBinding:test_insert_multiline()
        assertEquals( bufferContent(), "" )
        local s = "coucou\nhop"
        insert(1,1,s)
        assertEquals( bufferContent(), s )
    end

    function TestLuaBinding:test_insertline()
        local s1,s2,s3
        s1 = "1111"
        s2 = "2222"
        s3 = "3333"

        assertEquals( bufferContent(), "" )
        insertline(1,s2)
        assertEquals( bufferContent(), s2 )
        insertline(1,s1)
        assertEquals( bufferContent(), s1.."\n"..s2 )
        insertline(3,s3)
        assertEquals( bufferContent(), s1.."\n"..s2.."\n"..s3 )

        insertline(5,s3)
        assertEquals( bufferContent(), s1.."\n"..s2.."\n"..s3 )

        -- multiline support
        self:setUp()
        insertline(1,s3)
        assertEquals( bufferContent(), s3 )
        insertline(1,s1.."\n"..s2)
        assertEquals( bufferContent(), s1.."\n"..s2.."\n"..s3 )
    end

    function TestLuaBinding:test_appendline()
        assertEquals( bufferContent(), "" )
        local s = "coucou"
        appendline(s)
        assertEquals( bufferContent(), s )
        appendline("hop\nbof")
        assertEquals( bufferContent(), s.."\nhop\nbof" )
    end

    function TestLuaBinding:test_replace()
        local s = "coucou"
        appendline(s)
        assertEquals( bufferContent(), s )
        replace(1,1,"b")
        assertEquals( bufferContent(), "boucou" )
        replace(1,1,"doi")
        assertEquals( bufferContent(), "doicou" )
        replace(6,1,"hop")
        assertEquals( bufferContent(), "doicohop" )
        replace(9,1,"bof")
        assertEquals( bufferContent(), "doicohopbof" )

        -- ignore replace on wrong position
        replace(13,1,"bof")
        assertEquals( bufferContent(), "doicohopbof" )
        replace(1,3,"bof")
        assertEquals( bufferContent(), "doicohopbof" )

        -- replace on the second line
        replace(2,2,"bof")
        assertEquals( bufferContent(), "doicohopbof\nbof" )

        -- replace multiline rejected
        replace(2,1,"bof\nhop\n")
        assertEquals( bufferContent(), "doicohopbof\nbof" )
    end

        



luaUnit:run()

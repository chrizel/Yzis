--[[

Description: Test all the funcitons of the lua binding by calling them at
least once and checking the result type.

Author: Philippe Fremy
Version: 0.1
License: LGPL

]]--


-- Unit testing starts
require('luaunit')
require('utils')

TestLuaBinding = {} --class
    function TestLuaBinding:setUp() 
        clearBuffer()
        sendkeys("<ESC>")
    end

    function TestLuaBinding:tearDown() 
        clearBuffer()
    end

    function TestLuaBinding:test_setup()
        -- check that setup correctly clears the buffer
        insert(1,1,"coucou")
        assertEquals(line(1), "coucou" )
        self:setUp()
        assertEquals(line(1), "" )
        assertEquals( bufferContent(), "" )
    end

--    function TestLuaBinding:test_use_all_functions() -- disabled for now
--		require('../../doc/examples/example')
--    end

    function TestLuaBinding:test_version()
        -- ok, simple one
        s = version()
        assertEquals( string.len(s) > 0, true )

        -- bad number of arguments
        assertError( version, 1 )
    end

    function TestLuaBinding:test_linecount()
        assertEquals( 1, linecount() )
        insert(1,1,"coucou")
        -- still only one line
        assertEquals( 1, linecount() )
        appendline("hop")
        assertEquals( 2, linecount() )

        -- bad number of arguments
        assertError( linecount, 1 )
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

        -- bad number of arguments
        assertError( insert, 1, 2 )
        assertError( insert, 1, 2, 'coucou', 3 )
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
        clearBuffer()
        insertline(1,s3)
        assertEquals( bufferContent(), s3 )
        insertline(1,s1.."\n"..s2)
        assertEquals( bufferContent(), s1.."\n"..s2.."\n"..s3 )

        -- bad number of arguments
        assertError( insertline, 1 )
        assertError( insertline, 1, 'coucou', 3 )
    end

    function TestLuaBinding:test_appendline()
        assertEquals( bufferContent(), "" )
        local s = "coucou"
        appendline(s)
        assertEquals( bufferContent(), s )
        appendline("hop\nbof")
        assertEquals( bufferContent(), s.."\nhop\nbof" )

        -- bad number of arguments
        assertError( appendline )
        assertError( appendline, 'test', 1 )
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

        -- bad number of arguments
        assertError( replace, 1, 2 )
        assertError( replace, 1, 2, 'text', 1 )
    end

    function TestLuaBinding:test_deleteline()
        appendline("1")
        appendline("2")
        appendline("3")
        appendline("4")
        appendline("5")
        assertEquals( bufferContent(), "1\n2\n3\n4\n5" )

        deleteline(6)
        assertEquals( bufferContent(), "1\n2\n3\n4\n5" )
        deleteline(0)
        assertEquals( bufferContent(), "2\n3\n4\n5" )
        deleteline(2)
        assertEquals( bufferContent(), "2\n4\n5" )

        -- bad number of arguments
        assertError( deleteline )
        assertError( deleteline, 1, 2 )
    end


    -- disabled until it works        
    function TestLuaBinding:Xtest_filename()
        f1 = filename()
        print("filename: "..f1)
        assertEquals( string.len(f1) > 0, true )
        sendkeys( ':e! toto.txt<ENTER>' )
        f2 = filename()
        print("filename: "..f2)
        assertEquals( string.len(f2) > 0, true )
        if os.getenv('CYGWIN') then
            assert(string.find(f2,'toto.txt'))
        else
            assertEquals( f2, os.getenv('PWD')..'/toto.txt' )
        end
        assertEquals( f1 ~= f2, true )
        sendkeys( ':bd!<ENTER>' )

        -- bad number of arguments
        assertError( filename, 1 )
    end

    function TestLuaBinding:test_goto_and_pos()
        clearBuffer()
        assertEquals( winline(), 1 )
        assertEquals( wincol(), 1 )
        c,l = winpos(); assertEquals( l, 1); assertEquals( c, 1)

        appendline("111")
        appendline("222")
        appendline("333")
        goto(1,1)
        assertEquals( winline(), 1 )
        assertEquals( wincol(), 1 )
        c,l = winpos(); assertEquals( l, winline() ); assertEquals( c, winpos() )

        goto(2,1)
        assertEquals( wincol(), 2 )
        assertEquals( winline(), 1 )
        c,l = winpos(); assertEquals( l, winline() ); assertEquals( c, winpos() )

        goto(1,2)
        assertEquals( wincol(), 1 )
        assertEquals( winline(), 2 )
        c,l = winpos(); assertEquals( l, winline() ); assertEquals( c, winpos() )

        goto(2,2)
        assertEquals( wincol(), 2 )
        assertEquals( winline(), 2 )
        c,l = winpos(); assertEquals( l, winline() ); assertEquals( c, winpos() )

        goto(4,2)
        assertEquals( wincol(), 3 )
        assertEquals( winline(), 2 )
        c,l = winpos(); assertEquals( l, winline() ); assertEquals( c, winpos() )

        goto(2,4)
        assertEquals( wincol(), 2 )
        assertEquals( winline(), 3 )
        c,l = winpos(); assertEquals( l, winline() ); assertEquals( c, winpos() )

        goto(0,0)
        assertEquals( winline(), 1 )
        assertEquals( wincol(), 1 )
        c,l = winpos(); assertEquals( l, winline() ); assertEquals( c, winpos() )
        -- bad number of arguments
        assertError( goto, 1 )
        assertError( goto, 1, 2, 3 )
    end

    -- disabled until it works
    function TestLuaBinding:Xtest_color()
        sendkeys(':e ../files/color.h<ENTER>')
        color1 = color(1,1)
        print("color1 : "..color1)
        color2 = color(47,2)
        print("color2 : "..color2)
        assertEquals( string.len(color1) > 0, true )
        assertEquals( string.len(color2) > 0, true )
        assertEquals( color1 ~= color2, true )
        sendkeys(':bd!<ENTER>')

        -- bad number of arguments
        assertError( color, 1 )
        assertError( color, 1, 2, 3 )
    end

    function TestLuaBinding:test_setline()
        assertEquals( bufferContent(), "" )
        setline(1, "hop")
        assertEquals( bufferContent(), "hop" )
        setline(1, "coucou")
        assertEquals( bufferContent(), "coucou" )

        appendline("bof")
        assertEquals( bufferContent(), "coucou\nbof" )
        setline(2, "a")
        assertEquals( bufferContent(), "coucou\na" )

        setline(3, "hop")
        assertEquals( bufferContent(), "coucou\na" )

        -- bad number of arguments
        assertError( setline, 1 )
        assertError( setline, 1, 'text', 3 )
    end

    function TestLuaBinding:test_mode()
        sendkeys("<ESC>")
        assertEquals( mode(), MODE_NORMAL )
        sendkeys("icoucou")
        assertEquals( mode(), MODE_INSERT )
        sendkeys("<ESC>")
    end



if not _REQUIREDNAME then
    -- ret = LuaUnit:run('TestLuaBinding:test_filename') -- will execute only one test
    ret = LuaUnit:run() -- will execute all tests
    setLuaReturnValue( ret )
end


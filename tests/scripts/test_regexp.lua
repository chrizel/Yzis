
require('luaunit')
--require('regexp')
require('utils')


TestRegexp = {} --class
    function TestRegexp:setUp() 
    end

    function TestRegexp:tearDown() 
    end

	function TestRegexp:test_simple_usage()
        re = Regexp.create( 'toto' )
        assertEquals( re:match('toto'), 0 )
        assertEquals( re:match('titi'), -1 )
	end

	function TestRegexp:test_two_re()
        re1 = Regexp.create( 'toto' )
        re2 = Regexp.create( 'titi' )

        assertEquals( re1:match('toto'), 0 )
        assertEquals( re1:match('titi'), -1 )

        assertEquals( re2:match('titi'), 0 )
        assertEquals( re2:match('toto'), -1 )
	end

	function TestRegexp:test_try_to_force_finalize()
        for i = 0,100 do
            re = Regexp.create( 'toto' )
        end
	end

    function TestRegexp:Xtest_display_regexp_content()
        re1 = Regexp.create( 'toto' )
        print("Re object: ")
        table.foreach( re1, print )
        print("Re metatable: ")
        table.foreach( getmetatable(re1), print )
        print("Re.userdata metatable: ")
        table.foreach( getmetatable(re1['qregexp*']), print )
    end

    function TestRegexp:test_real_regexp()
        -- digit matching
        re = Regexp.create( '\\d' )
        assertEquals( re:match( 'aa 1' ), 3 )

        -- non digit matching
        re = Regexp.create( '\\D' )
        assertEquals( re:match( '111 1a2' ), 3 )

        -- dot (any character) matching
        re = Regexp.create( '11.' )
        assertEquals( re:match( 'aaa112' ), 3 )

        -- any whitespace
        re = Regexp.create( '\\s' )
        assertEquals( re:match( 'aaa 12' ), 3 )
        assertEquals( re:match( 'aaa\t12' ), 3 )
        assertEquals( re:match( 'aaa\n12' ), 3 )

        -- non whitespace
        re = Regexp.create( '\\S' )
        assertEquals( re:match( '   112' ), 3 )
        assertEquals( re:match( '\t\t\t112' ), 3 )

        -- word character: [a-zA-Z_]
        re = Regexp.create( '\\w' )
        assertEquals( re:match( '   112' ), 3 )
        assertEquals( re:match( '   t12' ), 3 )
        assertEquals( re:match( '   _12' ), 3 )

        -- non word character
        re = Regexp.create( '\\W' )
        assertEquals( re:match( '123 ' ), 3 )
        assertEquals( re:match( '123\n' ), 3 )

        -- back reference
        re = Regexp.create( '(\\w)_(\\w)_(\\2)_(\\1)' )
        assertEquals( re:match( 'XX a_b_a_b ' ), -1 )
        assertEquals( re:match( 'XX a_b_b_a ' ), 3 )

        -- sets
        re = Regexp.create( '[a-c]' )
        assertEquals( re:match( 'ddda' ), 3 )
        re = Regexp.create( '[abc]' )
        assertEquals( re:match( 'ddda' ), 3 )
        re = Regexp.create( '[^abc]' )
        assertEquals( re:match( 'abcd' ), 3 )
        re = Regexp.create( '[^a-c]' )
        assertEquals( re:match( 'abcd' ), 3 )

        -- quantifiers
        re = Regexp.create( 'ba?' )
        assertEquals( re:match( 'accbaX' ), 3 )
        assertEquals( re:match( 'accbX' ), 3 )

        re = Regexp.create( 'ba+' )
        assertEquals( re:match( ' ccbaX' ), 3 )
        assertEquals( re:match( ' ccbaaX' ), 3 )
        assertEquals( re:match( ' ccbX' ), -1 )

        re = Regexp.create( 'ba*' )
        assertEquals( re:match( ' ccbX' ), 3 )
        assertEquals( re:match( ' ccbaX' ), 3 )
        assertEquals( re:match( ' ccbaaX' ), 3 )

        re = Regexp.create( 'x(ab){1}X' )
        assertEquals( re:match( '123xabX' ), 3 )
        assertEquals( re:match( '123xababX' ), -1 )
        assertEquals( re:match( '123xX' ), -1 )

        re = Regexp.create( 'x(ab){1,}X' )
        assertEquals( re:match( '123xabX' ), 3 )
        assertEquals( re:match( '123xababX' ), 3 )
        assertEquals( re:match( '123xX' ), -1 )

        re = Regexp.create( 'x(ab){,1}X' )
        assertEquals( re:match( '123xX' ), 3 )
        assertEquals( re:match( '123xabX' ), 3 )
        assertEquals( re:match( '123xababX' ), -1 )

        re = Regexp.create( 'x(ab){1,2}X' )
        assertEquals( re:match( '123xX' ), -1 )
        assertEquals( re:match( '123xabX' ), 3 )
        assertEquals( re:match( '123xababX' ), 3 )

        -- alternating
        re = Regexp.create( '1|2' )
        assertEquals( re:match( 'xxx123' ), 3 )
        assertEquals( re:match( 'xxx213' ), 3 )

        -- capturing
        re = Regexp.create( '(1|2)_\\1' )
        assertEquals( re:match( 'xxx1_1' ), 3 )
        assertEquals( re:match( 'xxx2_2' ), 3 )
        assertEquals( re:match( 'xxx1_2' ), -1 )

        -- non capturing
        re = Regexp.create( '(?:1|2)_(a)_\\1' )  -- \\1 refers to a
        assertEquals( re:match( 'xxx1_a_1' ), -1 )
        assertEquals( re:match( 'xxx1_a_a' ), 3 )

        -- positions
        re = Regexp.create( '^a' )
        assertEquals( re:match( 'xxxa' ), -1 )
        assertEquals( re:match( 'axxa' ), 0 )
    
        re = Regexp.create( 'a$' )
        assertEquals( re:match( 'axxx' ), -1 )
        assertEquals( re:match( 'axxa' ), 3 )
   
        -- word boundary 
        re = Regexp.create( '\\bhop\\b' )
        assertEquals( re:match( '---hop---' ),3 )
        assertEquals( re:match( '---hophop--' ),-1 )

        -- look ahead
        re = Regexp.create( 'const(?=\\s+char)' ) -- const followed by char
        assertEquals( re:match( 'const hop' ), -1 )
        assertEquals( re:match( 'const char' ), 0 )
   
        re = Regexp.create( 'const(?!\\s+char)' ) -- const not followed by char
        assertEquals( re:match( 'const hop' ), 0 )
        assertEquals( re:match( 'const char' ), -1 )
   

    end

if not _REQUIREDNAME then
    -- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
    -- LuaUnit:run('TestLuaBinding') -- will execute only one class of test
    LuaUnit:run() -- will execute all tests
end

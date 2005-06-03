
require('luaunit')
require('utils')

--[[
Regexp compatibility with Vim Regexp.

The regexp support of yzis is based on the QRegexp from Qt. See
http://doc.trolltech.com/ ...

QRegexp is compatible with the universal perl regexp. 

The supported options are quite extensive:
- ?, +, *, {a,b}, {a,}, {,b}
- [a-b], [^a-b], (?:xxx)
- \d, \D, \s, \S, \w, \W, \1, \2, ...
- ^, $, \w, \W, (?=xxx), (?!xxx)

The restrictions are (according to Trolltech):
- greediness can only be controlled on the re itself with the setMinimal
  method. Greediness can not be controlled on operator level
- ^ always means beginning of string, so must be always escaped when one means
  that character ^
- $ always means end of string, so must be always escaped when one means
  that character $
- no equivalent for $`, $' and $+
- /x is not supported
- (?i), (?#comment) are not supported

You can create regexp using the vim syntax by calling VimRegexp() instead of
Regexp() as the constructor.
Supported Vim regexp syntax:
- \*, \+, \=, \? 
- \{}, \{n,m}, ...

Non supported Vim regexp syntax:
- non greedy matching: \{-n,m}, \{-n,}, ... 
This can be adjusted globally on a regexp with setMinimal()


]]

TestRegexp = {} --class
    function TestRegexp:test_usage()
        re = Regexp('toto')
        assertEquals( re:matchIndex('aaatoto'), 3 )
        assertEquals( re:matchIndex('titi'), -1 )
        assert( re:match('aaatoto') )
        assert( not re:match('titi') )

        assertEquals( Regexp('toto'):matchIndex('aaatoto'), 3 )
        assertEquals( Regexp('toto'):match('aaatoto'), true )
    end

	function TestRegexp:test_two_re()
        re1 = Regexp( 'toto' )
        re2 = Regexp( 'titi' )

        assertEquals( re1:matchIndex('toto'), 0 )
        assertEquals( re1:matchIndex('titi'), -1 )

        assertEquals( re2:matchIndex('titi'), 0 )
        assertEquals( re2:matchIndex('toto'), -1 )
	end

	function TestRegexp:test_try_to_force_finalize()
        for i = 0,100 do
            re = Regexp( 'toto' )
        end
	end

    function TestRegexp:Xtest_display_regexp_content()
        print("Regexp table: ")
        table.foreach( Regexp, print )
        print("Regexp metatable: ")
        mt = getmetatable( Regexp )
        if mt
        then
            table.foreach( mt, print )
        else
            print("nil")
        end

        re1 = Regexp( 'toto' )
        print("Re object: ")
        table.foreach( re1, print )
        print("Re object metatable: ")
        mt = getmetatable(re1)
        if mt
        then
            table.foreach( mt , print )
        else
            print("nil")
        end
        print("Re object qregexp* metatable: ")
        table.foreach( getmetatable(re1['qregexp*']), print )
    end

    function TestRegexp:test_real_regexp()
        -- digit matching
        re = Regexp( '\\d' )
        assertEquals( re:matchIndex( 'aa 1' ), 3 )

        -- non digit matching
        re = Regexp( '\\D' )
        assertEquals( re:matchIndex( '111 1a2' ), 3 )

        -- dot (any character) matching
        re = Regexp( '11.' )
        assertEquals( re:matchIndex( 'aaa112' ), 3 )

        -- any whitespace
        re = Regexp( '\\s' )
        assertEquals( re:matchIndex( 'aaa 12' ), 3 )
        assertEquals( re:matchIndex( 'aaa\t12' ), 3 )
        assertEquals( re:matchIndex( 'aaa\n12' ), 3 )

        -- non whitespace
        re = Regexp( '\\S' )
        assertEquals( re:matchIndex( '   112' ), 3 )
        assertEquals( re:matchIndex( '\t\t\t112' ), 3 )

        -- word character: [a-zA-Z_]
        re = Regexp( '\\w' )
        assertEquals( re:matchIndex( '   112' ), 3 )
        assertEquals( re:matchIndex( '   t12' ), 3 )
        assertEquals( re:matchIndex( '   _12' ), 3 )

        -- non word character
        re = Regexp( '\\W' )
        assertEquals( re:matchIndex( '123 ' ), 3 )
        assertEquals( re:matchIndex( '123\n' ), 3 )

        -- back reference
        re = Regexp( '(\\w)_(\\w)_(\\2)_(\\1)' )
        assertEquals( re:matchIndex( 'XX a_b_a_b ' ), -1 )
        assertEquals( re:matchIndex( 'XX a_b_b_a ' ), 3 )

        -- sets
        re = Regexp( '[a-c]' )
        assertEquals( re:matchIndex( 'ddda' ), 3 )
        re = Regexp( '[abc]' )
        assertEquals( re:matchIndex( 'ddda' ), 3 )
        re = Regexp( '[^abc]' )
        assertEquals( re:matchIndex( 'abcd' ), 3 )
        re = Regexp( '[^a-c]' )
        assertEquals( re:matchIndex( 'abcd' ), 3 )

        -- quantifiers
        re = Regexp( 'ba?' )
        assertEquals( re:matchIndex( 'accbaX' ), 3 )
        assertEquals( re:matchIndex( 'accbX' ), 3 )

        re = Regexp( 'ba+' )
        assertEquals( re:matchIndex( ' ccbaX' ), 3 )
        assertEquals( re:matchIndex( ' ccbaaX' ), 3 )
        assertEquals( re:matchIndex( ' ccbX' ), -1 )

        re = Regexp( 'ba*' )
        assertEquals( re:matchIndex( ' ccbX' ), 3 )
        assertEquals( re:matchIndex( ' ccbaX' ), 3 )
        assertEquals( re:matchIndex( ' ccbaaX' ), 3 )

        re = Regexp( 'x(ab){1}X' )
        assertEquals( re:matchIndex( '123xabX' ), 3 )
        assertEquals( re:matchIndex( '123xababX' ), -1 )
        assertEquals( re:matchIndex( '123xX' ), -1 )

        re = Regexp( 'x(ab){1,}X' )
        assertEquals( re:matchIndex( '123xabX' ), 3 )
        assertEquals( re:matchIndex( '123xababX' ), 3 )
        assertEquals( re:matchIndex( '123xX' ), -1 )

        re = Regexp( 'x(ab){,1}X' )
        assertEquals( re:matchIndex( '123xX' ), 3 )
        assertEquals( re:matchIndex( '123xabX' ), 3 )
        assertEquals( re:matchIndex( '123xababX' ), -1 )

        re = Regexp( 'x(ab){1,2}X' )
        assertEquals( re:matchIndex( '123xX' ), -1 )
        assertEquals( re:matchIndex( '123xabX' ), 3 )
        assertEquals( re:matchIndex( '123xababX' ), 3 )

        -- alternating
        re = Regexp( '1|2' )
        assertEquals( re:matchIndex( 'xxx123' ), 3 )
        assertEquals( re:matchIndex( 'xxx213' ), 3 )

        -- capturing
        re = Regexp( '(1|2)_\\1' )
        assertEquals( re:matchIndex( 'xxx1_1' ), 3 )
        assertEquals( re:matchIndex( 'xxx2_2' ), 3 )
        assertEquals( re:matchIndex( 'xxx1_2' ), -1 )

        -- non capturing
        re = Regexp( '(?:1|2)_(a)_\\1' )  -- \\1 refers to a
        assertEquals( re:matchIndex( 'xxx1_a_1' ), -1 )
        assertEquals( re:matchIndex( 'xxx1_a_a' ), 3 )

        -- positions
        re = Regexp( '^a' )
        assertEquals( re:matchIndex( 'xxxa' ), -1 )
        assertEquals( re:matchIndex( 'axxa' ), 0 )
    
        re = Regexp( 'a$' )
        assertEquals( re:matchIndex( 'axxx' ), -1 )
        assertEquals( re:matchIndex( 'axxa' ), 3 )
   
        -- word boundary 
        re = Regexp( '\\bhop\\b' )
        assertEquals( re:matchIndex( '---hop---' ),3 )
        assertEquals( re:matchIndex( '---hophop--' ),-1 )

        -- look ahead
        re = Regexp( 'const(?=\\s+char)' ) -- const followed by char
        assertEquals( re:matchIndex( 'const hop' ), -1 )
        assertEquals( re:matchIndex( 'const char' ), 0 )
   
        re = Regexp( 'const(?!\\s+char)' ) -- const not followed by char
        assertEquals( re:matchIndex( 'const hop' ), 0 )
        assertEquals( re:matchIndex( 'const char' ), -1 )
    end

    function TestRegexp:test_regexp_options()
        -- insensitiveness
        re = Regexp( 'a+' )
        assertEquals( re:matchIndex( 'xxxAAA' ), -1 )
        re:setCaseSensitive( false )
        assertEquals( re:matchIndex( 'xxxAAA' ), 3 )

        -- greediness
        re = Regexp( 'a+' )
        assertEquals( re:matchIndex( '   aaa' ), 3 )
        assertEquals( re:captured(0), 'aaa' )
        re:setMinimal( true )
        assertEquals( re:matchIndex( '   aaa' ), 3 )
        assertEquals( re:captured(0), 'a' )
    end

    function TestRegexp:test_capture_info()
        re = Regexp( 'a+' )
        assertEquals( re:matchIndex( '   aaa' ), 3 )
        assertEquals( re:captured(0),  'aaa' )
        assertEquals( re:captured(1),  nil )
        assertEquals( re:numCaptures(),  0 )
        assertEquals( re:pos(0),  3 )
        assertEquals( re:pos(1),  -1 )

        re = Regexp( '((a+)(b+))' )
        assertEquals( re:matchIndex( '   aabb' ), 3 )
        assertEquals( re:captured(0),  'aabb' )
        assertEquals( re:captured(1),  'aabb' )
        assertEquals( re:captured(2),  'aa' )
        assertEquals( re:captured(3),  'bb' )
        assertEquals( re:captured(4),  nil )
        assertEquals( re:numCaptures(),  3 )
        assertEquals( re:pos(0),  3 )
        assertEquals( re:pos(1),  3 )
        assertEquals( re:pos(2),  3 )
        assertEquals( re:pos(3),  5 )
        assertEquals( re:pos(4),  -1 )
    end

    function TestRegexp:test_vim_regexp()
        vre = VimRegexp('toto')
        assertEquals( vre:matchIndex('   toto'), 3 )

        assertEquals( VimRegexp('a\*b'):matchIndex('   ab'), 3 )
        assertEquals( VimRegexp('a\*b'):matchIndex('   aaab'), 3 )
        assertEquals( VimRegexp('a\*b'):matchIndex('   b'), 3 )
    end

    function TestRegexp:test_get_pattern()
        re = Regexp('toto')
        assertEquals( re:pattern(), 'toto' )
    end

    function TestRegexp:test_replace()
        re = Regexp('(aaa) (bbb) (ccc)')
        s = "aaa bbb ccc"
        result = re:replace( s, [[\3 \2 \1]] )
        expected = 'ccc bbb aaa'
        assertEquals( result, expected )

        re = Regexp('aaa')
        s = "aaa bbb aaa bbb aaa"
        result = re:replace( s, 'x' )
        expected = 'x bbb x bbb x'
        assertEquals( result, expected )

        result = re:replace( s, 'x', 1 )
        expected = 'x bbb aaa bbb aaa'
        assertEquals( result, expected )

        result = re:replace( s, 'x', 2 )
        expected = 'x bbb x bbb aaa'
        assertEquals( result, expected )

        result = re:replace( s, 'x', 3 )
        expected = 'x bbb x bbb x'
        assertEquals( result, expected )

        result = re:replace( s, 'x', 4 )
        expected = 'x bbb x bbb x'
        assertEquals( result, expected )

        s = 'aaaa'
        result = Regexp('aa'):replace( s, 'XX' )
        expected = 'XXXX'
        assertEquals( result, expected )

        -- no change in case of failure
        result = Regexp('XXX'):replace( s, '', 4 )
        expected = s
        assertEquals( result, expected )

    end

if not _REQUIREDNAME then
    -- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
    -- LuaUnit:run('TestLuaBinding') -- will execute only one class of test
    LuaUnit:run() -- will execute all tests
end

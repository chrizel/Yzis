-- call me from yzis with :source file.lua 
-- or directly with: yzis -c ':source file.lua'

-- Unit testing starts
require('luaunit')

--
-- delete the first line
--delete(1)
--insert(1,1,"See how Lua rocks !")
--replace(1,2,"Lua definitely rocks, we should just replace that text\nAnd we do it on multiple lines")
--goto(2,1)

--[[
text
line
insert
replace
wincol
winline
goto
delete
version
]]--


TestMyStuff = {} --class
    function TestMyStuff:testWithNumbers()
        a = 1
        b = 2
        result = my_super_function( a, b )
        assertEquals( type(result), 'number' )
        assertEquals( result, 3 )
    end

    function TestMyStuff:testWithRealNumbers()
        a = 1.1
        b = 2.2
        result = my_super_function( a, b )
        assertEquals( type(result), 'number' )
        -- I would like the result to be always rounded to an integer
        -- but it won't work with my simple implementation
        -- thus, the test will fail
        assertEquals( result, 3 )
    end

-- class TestMyStuff

luaUnit:run()

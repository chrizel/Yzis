--[[

Description: Test the utils.lua script.

Author: Philippe Fremy
Version: 0.1
License: LGPL

]]--

require('utils')
require('luaunit')
require('vimregexp')

TestUtils = {}
function TestUtils:testInTable()
    t = {}
    t['a'] = '1'
    t['b'] = '2'
    assertEquals( inTable(t, 'X', 'Y' ), nil )
    assertEquals( inTable(t, 'a', 'Y' ), 'a')
    assertEquals( inTable(t, 'Y', 'b' ), 'b')
end

function TestUtils:testTokenizer()
    tokens = {}
    tokens['aa'] = true
    tokens['bb'] = true
    s = '1aXaaXbXaaa'
    expected = {'1', 'a', 'X', 'aa', 'X', 'b', 'X', 'aa', 'a' }
    result = tokenizer( tokens, s )
    --print("Result:")
    --table.foreachi( result, print )
    table.foreachi( expected, function(idx,v)
        assertEquals( result[idx], v )            
        end 
    )
end

if not _REQUIREDNAME then
     ret = LuaUnit:run() -- will execute all tests
    setLuaReturnValue( ret )
end

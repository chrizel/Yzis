
require('luaunit')
require('test_bugs1')
require('test_excommands')
require('test_lua_binding')
require('test_utils')
require('test_movements')
require('test_search')
require('test_regexp')
require('test_vim_pattern')

-- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
-- LuaUnit:run('TestMovements') -- will execute only one class of test
LuaUnit:run() -- will execute all tests


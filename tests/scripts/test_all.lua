
require('luaunit')
require('test_lua_binding')
require('test_movements')
require('test_excommands')

-- LuaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
-- LuaUnit:run('TestMovements') -- will execute only one class of test
LuaUnit:run() -- will execute all tests


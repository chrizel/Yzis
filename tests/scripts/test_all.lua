
require('luaunit')
require('test_lua_binding')

-- luaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
-- luaUnit:run('TestLuaBinding') -- will execute only one class of test
luaUnit:run() -- will execute all tests


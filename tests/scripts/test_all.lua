
require('luaunit')
require('test_lua_binding')
require('test_movements')

-- luaUnit:run('TestLuaBinding:test_setline') -- will execute only one test
-- luaUnit:run('TestMovements') -- will execute only one class of test
luaUnit:run() -- will execute all tests


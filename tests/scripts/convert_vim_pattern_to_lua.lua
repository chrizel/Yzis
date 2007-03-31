--[[ =======================================================================

Script to convert the test_vim_pattern.vim file into a test_vim_pattern.lua
file.

Author : Philippe Fremy
License: LGPL
Version: 0.1

======================================================================== ]]--


finput = 'test_vim_pattern.vim'
foutput = 'test_vim_pattern.lua'

VALIDATED = 34

io.output(foutput)
test_nb = 0
function_started = nil
io.write(
[[--	Autogenerated test script from ']]..finput..[['

--	do not modify by hand

require('luaunit')
require('utils')

TestVimRegexp = {} --class
]] )

for line in io.lines(finput) do
	match, index, title = string.find(line, [[^echo%s*'(.+)']] )
	if match then
		test_nb = test_nb + 1
		if function_started then 
			io.write("\tend\n") 
		end
		function_started = 1
		io.write('\n\tfunction TestVimRegexp:test' .. string.format('%02d',test_nb) .. '()\n' )
		io.write("\t\t--" .. title .. "\n")
	end
	match, index, s, re, result = string.find(line,
		[[AssertEquals.%s*'(.*)'%s*=~%s*'(.*)'%s*.+(%d)]] )
	if match then
		if result == '1' then matchResult = 'true'
		else matchResult = 'false' end
		if string.sub(re,0,1) == '[' then
			re = "'" .. re .. "'"
		else
			re = "[[" .. re .. "]]"
		end
		io.write("\t\tassertEquals(VimRegexp("..re.."):match('"..s.."'), "..matchResult.." )\n" )
	end
	match, index, s = string.find(line, [[" unsupported (.*)]] )
	--print( tostring(match)..tostring(index)..tostring(s) )
	if match then
		io.write("\t\tassertError( VimRegexp, [[ "..tostring(s).." ]] )\n" )
	end
end
if function_started then io.write("\tend\n") end
io.write(
[[



if not _REQUIREDNAME then
]])
if false 
then
	for i = 1,test_nb do
		io.write("\t")
		if i > VALIDATED then io.write("--") end
		io.write("LuaUnit:run('TestVimRegexp:test"..string.format('%02d',i).."') -- will execute only one test\n")
	end
end
io.write(
[[
    LuaUnit:run() -- will execute all tests
end
]] )

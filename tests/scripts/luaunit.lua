--[[ 
		luaunit.lua

Description: A unit testing framework
Initial author: Ryu, Gwang (http://www.gpgstudy.com/gpgiki/LuaUnit)
improvements by Philippe Fremy <phil@freehackers.org>
Version: 1.1 

Changes between 1.1 and 1.0:
- internal variables are not global anymore
- you can choose between assertEquals( actual, expected) or assertEquals(
  expected, actual )
- you can assert for an error: assertError( f, a, b ) will assert that f(a,b)
  generates an error
- display the calling stack when an error is spotted


TODO:
- isolate output rendering into a separate class ?
- isolate result into a separate class ?
- customised output for the failing line ?
]]--

argv = arg

REVERSED_ASSERT_EQUALS = true

function assertError(f, ...)
	-- assert that calling f with the arguments will raise an error
	-- example: assertError( f, 1, 2 ) => f(1,2) should generate an error
	has_error, error_msg = not pcall( f, unpack(arg) )
	if has_error then return end 
	error( "No error generated", 2 )
end

function assertEquals(expected, actual)
	-- assert that two values are equal and calls error else
	if  actual ~= expected  then
		local function wrapValue( v )
			if type(v) == 'string' then return "'"..v.."'" end
			return tostring(v)
		end
		if REVERSED_ASSERT_EQUALS then
			expected, actual = actual, expected
		end

		local errorMsg = "expected: "..wrapValue(expected)..", actual: "..wrapValue(actual)
		error( errorMsg, 2 )
	end
end

assert_equals = assertEquals
assert_error = assertError

-------------------------------------------------------------------------------
luaUnit = {
	FailedCount = 0,
	TestCount = 0,
	Errors = {}
}

	function luaUnit:displayClassName( aClassName )
		print( '>>>>>> '..aClassName )
	end

	function luaUnit:displayTestName( testName )
		print( ">>> "..testName )
	end

	function luaUnit:displayFailure( errorMsg )
		print( errorMsg )
		print( 'Failed' )
	end

	function luaUnit:displaySuccess()
		print ("Ok" )
	end

	function luaUnit:displayResult()
		local failurePercent, successCount
		if self.TestCount == 0 then
			failurePercent = 0
		else
			failurePercent = 100.0 * self.FailedCount / self.TestCount
		end
		successCount = self.TestCount - self.FailedCount
		print( string.format("Success : %d%% - %d / %d",
			100-math.ceil(failurePercent), successCount, self.TestCount) )
    end

	-- Split text into a list consisting of the strings in text,
	-- separated by strings matching delimiter (which may be a pattern). 
	-- example: strsplit(",%s*", "Anna, Bob, Charlie,Dolores")
	function luaUnit.strsplit(delimiter, text)
		local list = {}
		local pos = 1
		if string.find("", delimiter, 1) then -- this would result in endless loops
			error("delimiter matches empty string!")
		end
		while 1 do
			local first, last = string.find(text, delimiter, pos)
			if first then -- found?
				table.insert(list, string.sub(text, pos, first-1))
				pos = last+1
			else
				table.insert(list, string.sub(text, pos))
				break
			end
		end
		return list
	end

	function luaUnit.isFunction(aObject)
		if  'function' == type(aObject) then
			return true
		else
			return false
		end
	end

	function luaUnit.strip_luaunit_stack(stack_trace)
		stack_list = luaUnit.strsplit( "\n", stack_trace )
		strip_end = nil
		for i = table.getn(stack_list),1,-1 do
			if string.find(stack_list[i],"[C]: in function `xpcall'",0,true)
				then
				strip_end = i - 2
			end
		end
		if strip_end then
			table.setn( stack_list, strip_end )
		end
		stack_trace = table.concat( stack_list, "\n" )
		return stack_trace
	end

    function luaUnit:runTestMethod(aName, aClassInstance, aMethod)
		local ok, errorMsg
		-- example: runTestMethod( 'TestToto:test1', TestToto, TestToto.testToto(self) )
		luaUnit:displayTestName(aName)
        self.TestCount = self.TestCount + 1

		-- run setUp first(if any)
		if self.isFunction( aClassInstance.setUp) then
				aClassInstance:setUp()
		end

		function err_handler(e)
			return e..'\n'..debug.traceback()
		end

		-- run testMethod()
        ok, errorMsg = xpcall( aMethod, err_handler )
		if not ok then
			errorMsg  = self.strip_luaunit_stack(errorMsg)
            self.FailedCount = self.FailedCount + 1
			table.insert( self.Errors, errorMsg )
			luaUnit:displayFailure( errorMsg )
        else
			luaUnit:displaySuccess()
        end

		-- lastly, run tearDown(if any)
		if self.isFunction(aClassInstance.tearDown) then
				 aClassInstance:tearDown()
		end
    end

	function luaUnit:runTestMethodName( methodName, classInstance )
		-- example: runTestMethodName( 'TestToto:testToto', TestToto )
		local methodInstance = loadstring(methodName .. '()')
		luaUnit:runTestMethod(methodName, classInstance, methodInstance)
	end

    function luaUnit:runTestClassByName( aClassName )
		-- example: runTestMethodName( 'TestToto' )
		local hasMethod, methodName, classInstance
		hasMethod = string.find(aClassName, ':' )
		if hasMethod then
			methodName = string.sub(aClassName, hasMethod+1)
			aClassName = string.sub(aClassName,1,hasMethod-1)
		end
        classInstance = _G[aClassName]
		if not classInstance then
			error( "No such class: "..aClassName )
		end
		luaUnit:displayClassName( aClassName )

		if hasMethod then
			if not classInstance[ methodName ] then
				error( "No such method: "..methodName )
			end
			luaUnit:runTestMethodName( aClassName..':'.. methodName, classInstance )
		else
			-- run all test methods of the class
			for methodName, method in classInstance do
				if luaUnit.isFunction(method) and string.sub(methodName, 1, 4) == "test" then
					luaUnit:runTestMethodName( aClassName..':'.. methodName, classInstance )
				end
			end
		end
		print()
	end

	function luaUnit:run(...)
		-- Run some specific test classes.
		-- If no arguments are passed, run the class names specified on the
		-- command line. If no class name is specified on the command line
		-- run all classes whose name starts with 'Test'
		--
		-- If arguments are passed, they must be strings of the class names 
		-- that you want to run
		if arg.n > 0 then
			table.foreachi( arg, luaUnit.runTestClassByName )
		else 
			if argv and argv.n > 0 then
				table.foreachi(argv, luaUnit.runTestClassByName )
			else
				for var, value in _G do
					if string.sub(var,1,4) == 'Test' then 
						luaUnit:runTestClassByName(var)
					end
				end
			end
		end
		luaUnit:displayResult()
	end
-- class luaUnit

function wrapFunctions(...)
	local testClass, testFunction
	testClass = {}
	local function storeAsMethod(idx, testName)
		testFunction = _G[testName]
		testClass[testName] = testFunction
	end
	table.foreachi( arg, storeAsMethod )
	return testClass
end

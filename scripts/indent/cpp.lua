-- Indentation plugin for C++
-- Author : Mickael Marchand

function Indent_cpp(nbNextTabs,nbNextSpaces,nbPrevTabs,nbPrevSpaces,prevLine,nextLine)
	local nbtabs = nbPrevTabs
	local nbspaces = nbPrevSpaces
	local result = ""

	local st = string.sub(prevLine,-1,-1)
	if st == "{" or st == ":" or st == "(" then nbtabs=nbtabs+1 end
	
	result = result..string.rep("\t",nbtabs)..string.rep(" ",nbspaces)

	if string.sub(prevLine,-3,-1) == "/**" then 
		result = result.." * " 
	elseif string.find(prevLine,"^%s*\*%s.*") then
		result = result.."* " 
	end

	return result
end

-- char is the char which caused the INDENT_ON_KEY event
function Indent_OnKey_cpp(char,nbPrevTabs,nbPrevSpaces,nbCurTabs,nbCurSpaces,nbNextTabs,nbNextSpaces,curLine,prevLine,nextLine)
	--check we are at end of line
	if wincol() ~= string.len(curLine)+1 then
		return
	end

	if char == "}" then
		-- find the matching opening { and use the same indent

		-- or just remove 1 tab for now ;)
		if nbCurTabs > 0 then
			remove(0, winline(), 1)
			goto(string.len(curLine), winline())
		end
	end
end

connect ("INDENT_ON_ENTER", "Indent_cpp")
connect ("INDENT_ON_KEY", "Indent_OnKey_cpp")
setlocal ("indentkeys=}")


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
	if wincol() ~= string.len(curLine)+1 then
		return
	end

	local xpos, ypos = winpos()

	goto(wincol()-1, winline())
	if char == "}" then
		-- find the matching opening { and use the same indent
		local found, x, y = matchpair()
		if found == true then 
			if ( y == winline() ) then -- we are on the same line, dont change anything ;)
				goto(xpos,ypos)
				return
			end
			lin = line(y+1)
--			debug("(matched line :"..lin..")")
			local news = string.gsub(lin, "^(%s*)(.*)$", "%1")
--			debug("(indent of this line :"..news..")")
			local newcurline = string.gsub(curLine, "^(%s*)(.*)$", "%2")
--			debug("(chars of curline :"..newcurline..")")
			newcurline = news..newcurline
			setline(winline(), newcurline)
			goto(string.len(newcurline)+1, winline())
			return
		end
	end
	goto(xpos,ypos)
end

connect ("INDENT_ON_ENTER", "Indent_cpp")
connect ("INDENT_ON_KEY", "Indent_OnKey_cpp")
setlocal ("indentkeys=}")


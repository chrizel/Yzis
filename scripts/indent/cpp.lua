-- Indentation plugin for C++
-- Author : Mickael Marchand

function Indent_cpp(nbNextTabs,nbNextSpaces,nbPrevTabs,nbPrevSpaces,prevLine,nextLine)
	local nbtabs = nbPrevTabs
	local nbspaces = nbPrevSpaces

	local st = string.byte(prevLine,-1)
	if st == string.byte("{",1) or st == string.byte(":",1) or st == string.byte("(",1) then nbtabs=nbtabs+1 end

	-- we use tabs only for now
	return nbtabs, nbspaces
end

function nbChar(line, char)
	local length = string.len(line)

	local nb = 0

	for i=1,length do
		local c = string.byte(line,i)
		if c == string.byte(char,1) then nb=nb+1
		else break
		end
	end

	return nb
end

-- char is the char which caused the INDENT_ON_KEY event
function Indent_OnKey_Lua(char)
	--check we are at end of line

	local curLine = line(winline())
	local prevLine = line(winline()-1)
	local curNbTabs = nbChar(curLine, "\t")
	local prevNbTabs = nbChar(prevLine, "\t")

	if string.byte(char,1) == string.byte("{",1) then
		if curNbTabs < prevNbTabs then
			local fill = string.rep("\t", prevNbTabs - curNbTabs + 1)
			insert(0,winline(),fill)
		else
			local fill = string.rep("\t", prevNbTabs - curNbTabs + 1)
			delete(0,winline(),curNbTabs - prevNbTabs + 1)
		end
	end

end

connect ("INDENT_ON_ENTER", "Indent_cpp")
-- connect ("INDENT_ON_KEY", "Indent_OnKey_cpp")


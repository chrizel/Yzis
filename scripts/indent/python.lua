-- Indentation plugin for python
-- Author : Loic Pauleve <panard@inzenet.org>

function Indent_python( nbNextTabs, nbNextSpaces, nbPrevTabs, nbPrevSpaces, prevLine, nextLine )
	local nbtabs = nbPrevTabs
	local nbspaces = nbPrevSpaces

	local result = string.rep( "\t", nbtabs )..string.rep( " ", nbspaces )
	
	if string.find( prevLine, ":%s*$" ) then
		result = result.."\t"
	end

	return result
end

connect( "INDENT_ON_ENTER", "Indent_python" )


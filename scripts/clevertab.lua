-- $Id$
-- Author : Mickael Marchand <mikmak@yzis.org>


function cleverTab(key) 
	local str = line(winline())
--	debug("("..str..")")
	
	local c = string.sub(str, wincol()-1, wincol()-1)
	if c == " " or c == "\t" or str == "" or wincol() == 1 then
		return "<TAB>"
	else
		return "<CTRL>p"
	end
	return ""
end 

imap("<TAB>", "<Script>cleverTab()")



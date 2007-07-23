-- Author : Mickael Marchand <mikmak@yzis.org>


function cleverTab(key) 
	local str = line(winline())
--	debug("("..str..")")
	
	local c = string.sub(str, wincol()-1, wincol()-1)
	if c == " " then
		-- remove the space and put a TAB instead, #144
		remove(wincol()-1, winline(), 1)
		return "<TAB>"
	end
	if c == "\t" or str == "" or wincol() == 1 then
		return "<TAB>"
	else
		return "<CTRL>p"
	end
	return ""
end 

imap("<TAB>", "<Script>cleverTab()")



-- $id$
-- Author : Mickael Marchand <mikmak@yzis.org>


function cleverTab(key) 
	local str = line(winline())
	if string.sub(str, -1,1) == " " or string.sub(str,-1,1) == "\t" or str=="" then
		return "<TAB>"
	else
		return "<CTRL>x<CTRL>n"
	end
	return ""
end 

imap("<TAB>", "<Script>cleverTab()")



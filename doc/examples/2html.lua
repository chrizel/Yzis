-- call me with :source 2html.lua from Yzis ;)
--
--

local fgc = "#ffffff"
local bgc = "#000000"
local encoding = "UTF-8"

function HtmlColor(c)
	return color(c);
end

function HtmlOpening(id)
	local a = ""
	if inverse then
	else
		local x = HtmlBgColor(id)
		if x ~= "" then
			a = a..'<span style="background-color: '..x..'">'
		end
		local x = HtmlColor(id)
		if x ~= "" then
			a = a..'<font color="'..x..'">'
		end
	end
	-- todo bold,italic,underline
	
	return a
end

function HtmlClosing(id)
	local a = ""
	-- TODO underline, bold,italic
	if inverse then
		a = a..'</font></span>'
	else
		local x = HtmlColor(id)
		if x ~= "" then
			a = a..'</font>'
		end
		x = HtmlBgColor(id)
		if x ~= "" then
			a = a..'</span>'
		end
	end
	return a
end

function CSS1(id)
	local a = ""
	if inverse then
		local x = HtmlBgColor(id)
		a = a.."color : "
		if x ~= "" then
			a = a..x
		else
			a = a..bgc
		end
		a = a.."; "
		x = HtmlColor(id)
		a = a.."background-color : "
		if x ~= "" then
			a = a..x
		else
			a = a..fgc
		end
		a = a.."; "
	else
		x = HtmlColor(id)
		if x ~= "" then
			a = a.."color :"..x.."; "
		end
		x = HtmlBgColor(id)
		if x ~= "" then
			a = a.."background-color : "..x.."; "
		end
		-- TODO bold,underline,italic
	end
end

-- start the real work
local file = filename()..".html"
local f = io.open(file,"w+")
local tag_close = ">"

if use_xhtml then
	f:write("<?xml version=\"1.0\"?>\n")
	tag_close = "/>"
end

f:write("<html>\n<head>\n<title>"..filename().."<title>\n")
f:write("<meta name=\"Generator\" content=\"Yzis "..version().."\""..tag_close.."\n")

if encoding ~= "" then
	f:write("<meta http-equiv=\"content-type\" content=\"text/html; charset="..encoding.."\""..tag_close.."\n")
end

if html_use_css then
	f:write("<style type=\"text/css\">\n<!--\n-->\n</style>\n")
end

if html_no_pre then
	f:write("</head>\n<body>\n")
else
	f:write("</head>\n<body>\n<pre>\n")
end

local idlist = ","
local expandedtab  = "        "
local lnum = 1
local lend = linecount()

while lnum < lend do
	local l = line(lnum)
	local len = string.len(l)
	local new = ""

	local col = 1
	while col < len do
		local startcol = col
		local id = HtmlColor(col)
		col = col + 1
		while col < len and id == HtmlColor(col) do
			col = col + 1
		end
		local id_name = id
		new = new..'<span class="'..id_name..'">'..l..'</span>'
		if not idlist.find(id_name) then
			idlist = idlist..id_name..","
		end

		if col > len then
			break
		end
	end

	local pad = 0

end


f:flush()
f:close()


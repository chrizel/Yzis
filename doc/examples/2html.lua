-- call me with :source 2html.lua from Yzis ;)
--
--

local fgc = "#ffffff"
local bgc = "#000000"
local encoding = "UTF-8"
html_use_css = true

function HtmlColor(c)
	return fgc
end

function HtmlBgColor(c)
	return bgc
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
		local x = id -- HtmlBgColor(id)
		a = a.."color : "
		if x ~= "" then
			a = a..x
		else
			a = a..bgc
		end
		a = a.."; "
		x = id -- HtmlColor(id)
		a = a.."background-color : "
		if x ~= "" then
			a = a..x
		else
			a = a..fgc
		end
		a = a.."; "
	else
		x = id -- HtmlColor(id)
		if x ~= "" then
			a = a.."color :"..x.."; "
		end
		x = id -- HtmlBgColor(id)
		if x ~= "" then
			a = a.."background-color : "..bgc.."; "
		end
		-- TODO bold,underline,italic
	end
	return a
end


-- start the real work
local file = filename()..".html"
local tag_close = ">"
storage = {}
local lines = 0
local idlist = ","
local expandedtab  = "        " --default to 8 for now
local lnum = 1
local lend = linecount()

function write(text) 
	storage[lines] = text
	lines = lines + 1
end


while lnum < lend do
	local l = line(lnum)
	local len = string.len(l)
	local new = ""

	local col = 1
	local startcol = 1

	while col <= len do
		startcol = col
		local id = color(col,lnum)
		col = col + 1
		while col <= len and id == color(col,lnum) do
			col = col + 1
		end
		new = new..'<span class="'..string.sub(id,2)..'">'..string.sub(l,startcol,col-1)..'</span>'
		if not string.find(idlist,id) then
			idlist = idlist..id..","
		end

		if col > len then
			break
		end
	end

	-- expand tabs
	local pad = 0
	local start = 0
	local idx = string.find(l,"\t")
	while idx ~= nil do
		local i = 8 - ( math.mod(start + pad + idx, 8) )
		new = string.gsub(new,"\t",string.sub(expandedtab, 0, i))
		pad = pad + i - 1
		start = start + idx + 1
		idx = string.find(string.sub(l,start),"\t")
	end
	
	write(new.."\n")
	lnum = lnum + 1
end

if html_no_pre then
	write("\n</body>\n</html>")	
else
	write("\n</pre>\n</body>\n</html>")	
end

function dump_styles()
	idlist = string.sub(idlist,1) -- remove first ','
	while idlist ~= "" do
		local attr = ""
		local col = string.find(idlist,",")
		local id = string.sub(idlist,1,col-1)
		idlist = string.sub(idlist,col+1)
		attr = CSS1(id)
		if attr ~= "" and attr ~= nil then
			if html_use_css then
				f:write("\n."..string.sub(id,2).." { "..attr.."}")
			else
				-- hmm ?
			end
		else
			-- hmm ?
		end
	end
end

function dump_main()
	for i = 0,lines do
		if storage[i] ~= nil then
			f:write(storage[i])
		end
	end
end


-- write to file
f = io.open(file,"w+")

if use_xhtml then
	f:write("<?xml version=\"1.0\"?>\n")
	tag_close = "/>"
end

if html_use_css then
	f:write("<DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n")
end
f:write("<html>\n<head>\n<title>"..filename().."</title>\n")
f:write("<meta name=\"Generator\" content=\"Yzis "..version().."\""..tag_close.."\n")

if encoding ~= "" then
	f:write("<meta http-equiv=\"content-type\" content=\"text/html; charset="..encoding.."\""..tag_close.."\n")
end

if html_use_css then
	f:write("<style type=\"text/css\"><!--\n")
	if html_no_pre then
		f:write("\nbody { color : "..fgc.."; background-color: "..bgc.."; font-family: Helvetica, Courier, monospace; }")
	else
		f:write("\npre { color : "..fgc.."; background-color: "..bgc.."; }")
		f:write("\nbody { color : "..fgc.."; background-color: "..bgc.."; }")
	end
	dump_styles()
	f:write("\n--></style>")

	if html_no_pre then
		f:write("</head>\n<body>\n")
	else
		f:write("</head>\n<body>\n<pre>\n")
	end
else
	if html_no_pre then
		f:write('</head>\n<body bgcolor="'..bgc..'" text="'..fgc..'" style="font-family: Courier, monospace;">\n')
	else
		f:write('</head>\n<body bgcolor="'..bgc..'" text="'..fgc..'">\n<pre>')
	end
end

dump_main()

f:flush()
f:close()


-- call me from yzis with :source file.lua 
-- or directly with: yzis -c ':source file.lua'

-- Use all the yzis lua functions

-- insert (tested)
for i = 1,10 do
    insert(1,i,"this is line "..i.."\n")
end

-- linecount() (tested)
insert(1,2,"This buffer has "..linecount().." lines" )

-- text, linecount 
--[[ not working
for i=1,linecount() do
    insert(1,linecount(),"This is the content of line "..i..", col "..i.." : "..text(i,i))
end
]]--

-- line (tested)
for i=1,10 do
    print("This is the content of line "..i..":")
    print(line(i))
end

-- replace (tested)
for i=1,10 do
    replace(9,i,"the replaced content of line "..i)
end

-- deleteline (tested)
deleteline(1)
deleteline(5)

-- version (tested)
print("Current version is : "..version())

-- filename (tested)
print("Current file is : "..filename())

-- wincol, winline (tested)
print("Current col,line: "..wincol()..","..winline())

-- goto (tested)
goto(3,1)
print("Current col,line: "..wincol()..","..winline())

-- color
--[[ not working
print("Current syntax color is : "..getcolor())
]]

-- sendkeys
sendkeys('i[this is inserted]<ENTER>')

-- print (lua functions)
print("Hello world!")


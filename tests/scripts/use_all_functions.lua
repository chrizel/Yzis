-- call me from yzis with :source file.lua 
-- or directly with: yzis -c ':source file.lua'

-- Use all the yzis lua functions

-- insert
for i = 1,10 do
    insert(1,i,"this is line "..i.."\n")
end

-- linecount()
insert(1,2,"This buffer has "..linecount().." lines" )

-- text, linecount 
--[[ not working
for i=1,linecount() do
    insert(1,linecount(),"This is the content of line "..i..", col "..i.." : "..text(i,i))
end
]]--

-- line
for i=1,10 do
    print("This is the content of line "..i..":")
    print(line(i))
end

-- replace
for i=1,10 do
    replace(9,i,"the replaced content of line "..i)
end

-- wincol, winline
print("Current col,line: "..wincol()..","..winline())

-- goto
goto(3,1)
print("Current col,line: "..wincol()..","..winline())

-- deleteline
deleteline(1)
deleteline(5)

-- filename
print("Current file is : "..filename())

-- version
print("Current version is : "..version())

-- color
--[[ not working
print("Current syntax color is : "..getcolor())
]]

-- sendkeys
sendkeys('i[this is inserted]<ENTER>')

-- print (lua functions)
print("Hello world!")


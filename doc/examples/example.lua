-- call me from yzis with :source example.lua 
-- or from the command line: yzis -c ':source example.lua<ENTER>'

-- Use all the yzis lua functions

-- appendline
for i = 1,10 do
    appendline("this is line "..i)
end

appendline("==========================================")

-- linecount()
appendline("This buffer has "..linecount().." lines" )

-- line 
for i=1,10 do
    appendline("This is the content of line "..i..":"..line(i))
end

-- insert 
insert(9,3,"(inserted in) ")
insert(9,4,"(inserted in) ")

-- replace 
replace(10,3,"replaced")

-- deleteline 
appendline("deleting line 6")
deleteline(6)

-- version 
appendline("Current version is : "..version())

-- filename 
appendline("Current file is : "..filename())

-- wincol, winline 
appendline("Current col,line: "..wincol()..","..winline())

-- goto 
goto(3,3)
appendline("Updated col,line: "..wincol()..","..winline())

-- color is not working
--[[
appendline("Current syntax color is : "..tostring(color()))
]]

-- sendkeys
sendkeys('i[this is inserted]<ENTER>')

-- print 
print("Hello world!")


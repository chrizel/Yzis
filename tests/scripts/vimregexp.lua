--[[ =======================================================================

Emulation of Vim Regexp syntax using lua regexp.

Author : Philippe Fremy
License: LGPL
Version: $Id$

======================================================================== ]]--

function inTable( t, ... )
    -- check whether the values passed in argument are keys of table t
    --print("InTable for table")
    --table.foreach(t, print)
    for i = 1,arg['n']
    do
        v = arg[i] 
        --print("\tChecking index "..tostring(i)..": '"..tostring(v).."'")
        --print ("t[v]="..tostring(t[v]))
        if t[v] then 
            --print("\tMatch for '"..v.."'")
            return v 
        end
    end
    --print "\tNo match"
    return nil
end

local __vimRegexpSubstTable = {}
__vimRegexpSubstTable['\\+'] = '+'
__vimRegexpSubstTable['\\='] = '?'
__vimRegexpSubstTable['\\?'] = '?'
__vimRegexpSubstTable['\\{'] = '{'
__vimRegexpSubstTable['\\|'] = '|'
__vimRegexpSubstTable['|'  ] = '\\|'
__vimRegexpSubstTable['\\('] = '('
__vimRegexpSubstTable['('  ] = '\\('
__vimRegexpSubstTable[')'  ] = '\\)'
__vimRegexpSubstTable['\\)'] = ')'
__vimRegexpSubstTable['\\<'] = '\\b'
__vimRegexpSubstTable['\\>'] = '\\b'
__vimRegexpSubstTable['\\x'] = '[0-9a-fA-F]'
__vimRegexpSubstTable['\\X'] = '[^0-9a-fA-F]'
__vimRegexpSubstTable['\\o'] = '[0-7]'
__vimRegexpSubstTable['\\O'] = '[^0-7]'
__vimRegexpSubstTable['\\h'] = '[a-zA-Z_]'
__vimRegexpSubstTable['\\H'] = '[^a-zA-Z_]'
__vimRegexpSubstTable['\\a'] = '[a-zA-Z]'
__vimRegexpSubstTable['\\A'] = '[^a-zA-Z]'
__vimRegexpSubstTable['\\l'] = '[a-z]'
__vimRegexpSubstTable['\\L'] = '[^a-z]'
__vimRegexpSubstTable['\\u'] = '[A-Z]'
__vimRegexpSubstTable['\\U'] = '[^A-Z]'
__vimRegexpSubstTable['[:alnum:]'] = 'a-zA-Z0-9'
__vimRegexpSubstTable['[:alpha:]'] = 'a-zA-Z'
__vimRegexpSubstTable['[:blank:]'] = '\\s'
__vimRegexpSubstTable['[:digit:]'] = '0-9'
__vimRegexpSubstTable['[:xdigit:]'] = '0-9A-Fa-f'
__vimRegexpSubstTable['[:lower:]'] = 'a-z'
__vimRegexpSubstTable['[:upper:]'] = 'A-Z'
__vimRegexpSubstTable['[:space:]'] = ' '
__vimRegexpSubstTable['\\%('] = '(?:'
__vimRegexpSubstTable['\\_^'] = '^'
__vimRegexpSubstTable['\\^'] = '\\^'
__vimRegexpSubstTable['\\_$'] = '$'
__vimRegexpSubstTable['\\$'] = '\\$'

local __vimRegexpUnsupported = {}
__vimRegexpUnsupported[ '[:return:]' ] = true
__vimRegexpUnsupported[ '[:tab:]'    ] = true
__vimRegexpUnsupported[ '[:graph:]' ] = true
__vimRegexpUnsupported[ '[:print:]' ] = true
__vimRegexpUnsupported[ '[:punct:]' ] = true
__vimRegexpUnsupported[ [[\&]] ] = true
__vimRegexpUnsupported[ [[{-]] ] = true
__vimRegexpUnsupported[ [[\@>]] ] = true
__vimRegexpUnsupported[ [[\@=]] ] = true
__vimRegexpUnsupported[ [[\@!]] ] = true
__vimRegexpUnsupported[ [[\@<=]] ] = true
__vimRegexpUnsupported[ [[\@<!]] ] = true
__vimRegexpUnsupported[ [[\zs]] ] = true
__vimRegexpUnsupported[ [[\ze]] ] = true
__vimRegexpUnsupported[ [[\_.]] ] = true
__vimRegexpUnsupported[ [[\%^]] ] = true
__vimRegexpUnsupported[ [[\%$]] ] = true
__vimRegexpUnsupported[ [[\%#]] ] = true
__vimRegexpUnsupported[ [[\%23l]] ] = true
__vimRegexpUnsupported[ [[\%23c]] ] = true
__vimRegexpUnsupported[ [[\%23v]] ] = true
__vimRegexpUnsupported[ [[\i]] ] = true
__vimRegexpUnsupported[ [[\I]] ] = true
__vimRegexpUnsupported[ [[\k]] ] = true
__vimRegexpUnsupported[ [[\K]] ] = true
__vimRegexpUnsupported[ [[\f]] ] = true
__vimRegexpUnsupported[ [[\F]] ] = true
__vimRegexpUnsupported[ [[\p]] ] = true
__vimRegexpUnsupported[ [[\P]] ] = true
__vimRegexpUnsupported[ [[\t]] ] = true
__vimRegexpUnsupported[ [[\e]] ] = true
__vimRegexpUnsupported[ [[\r]] ] = true
__vimRegexpUnsupported[ [[\b]] ] = true
__vimRegexpUnsupported[ [[\n]] ] = true
__vimRegexpUnsupported[ [[\~]] ] = true
__vimRegexpUnsupported[ [[\z1]] ] = true
__vimRegexpUnsupported[ [[\z9]] ] = true
__vimRegexpUnsupported[ [[\%[]] ] = true
__vimRegexpUnsupported[ [[\c]] ] = true
__vimRegexpUnsupported[ [[\C]] ] = true
__vimRegexpUnsupported[ [[\Z]] ] = true
__vimRegexpUnsupported[ [[\v]] ] = true
__vimRegexpUnsupported[ [[\V]] ] = true
__vimRegexpUnsupported[ [[\m]] ] = true
__vimRegexpUnsupported[ [[\M]] ] = true

local __vimRegexpTokens = {}
__vimRegexpTokens['\\^'] = true 
__vimRegexpTokens['[^'] = true 

table.foreach( __vimRegexpSubstTable, function(idx, val)
        __vimRegexpTokens[idx] = val
    end )

table.foreach( __vimRegexpUnsupported, function(idx, val)
        __vimRegexpTokens[idx] = val
    end )

function tokenizer( tokens, s )
    -- analyse s and returns a list of tokens or character
    ret = { n=0 }
    idx = 1
    while idx <= string.len(s)
    do
        c1 = string.sub(s,idx,idx)
        c2 = string.sub(s,idx,idx+1)
        c3 = string.sub(s,idx,idx+2)
        c4 = string.sub(s,idx,idx+3)
        c5 = string.sub(s,idx,idx+4)
        c6 = string.sub(s,idx,idx+5)
        c7 = string.sub(s,idx,idx+6)
        c8 = string.sub(s,idx,idx+7)
        c9 = string.sub(s,idx,idx+8)
        c10 = string.sub(s,idx,idx+9)
        c11 = string.sub(s,idx,idx+10)
        token = inTable( tokens, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11 ) or c1
        --print("token found: "..token)
        table.insert(ret, token)
        idx_inc = string.len(token)
        idx = idx + idx_inc
    end
    return ret
end


--[[
A class that supports Vim Regexp. Within Yzis, we normally use the class
Regexp that supports perl compatible regexp.

VimRegexp supports the same operators as Regexp but the Regexp is converted
from vim syntax to perl syntax before being used.
]]--
function VimRegexp( re )
    -- transform re
    --print("Before: '"..re.."'")
    local out = ''
    local tokenizedRe = tokenizer( __vimRegexpTokens, re )
    --print("Tokenized re")
    --table.foreach( tokenizedRe, print )
    for i = 1, tokenizedRe['n']
    do
        token = tokenizedRe[i]
        if __vimRegexpSubstTable[token]
        then
            out = out .. __vimRegexpSubstTable[token]        

        -- handle ^
        elseif (token == '^' and i ~= 1  
            and tokenizedRe[i-1] ~= '\\|'
            and tokenizedRe[i-1] ~= '\\(' )
        then
            out = out .. '\\^'

        -- handle $
        elseif (token == '$' and tokenizedRe[i+1] ~= nil  
            and tokenizedRe[i+1] ~= '\\|'
            and tokenizedRe[i+1] ~= '\\)' )
        then
            out = out .. '\\$'

        elseif __vimRegexpUnsupported[token]
        then
            error("Token "..token.." is not supported")
        else
            out = out .. token
        end
        --print("Partial Result: '"..out.."'")
    end
    --print("Result: '"..out.."'")
    return Regexp(out)
end


-- Split text into a list consisting of the strings in text,
-- separated by strings matching delimiter (which may be a pattern). 
-- example: strsplit(",%s*", "Anna, Bob, Charlie,Dolores")
function strsplit(delimiter, text)
    local list = {}
    local pos = 1
    if string.find("", delimiter, 1) then -- this would result in endless loops
        error("delimiter matches empty string!")
    end
    while 1 do
        local first, last = string.find(text, delimiter, pos)
        if first then -- found?
            table.insert(list, string.sub(text, pos, first-1))
            pos = last+1
        else
            table.insert(list, string.sub(text, pos))
            break
        end
    end
    return list
end


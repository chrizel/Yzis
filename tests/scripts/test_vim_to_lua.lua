--[[ 

Test the vim_to_lua converter with many extracts of the script vst.vim.

Description: A unit testing framework
Author: Philippe Fremy <phil@freehackers.org>
Version: 0.1

]]--


require('luaunit')
require('utils')
require('vim_to_lua')


TestVimToLua = {} --class
    function TestVimToLua:assertProcessStringList( l1, l2 )
        for i = 1,table.getn(l1) do
            lineL1 = l1[i]
            lineL2  = l2 [i]
            assertEquals( process_line(lineL1) , lineL2 )
        end
    end
    
    function TestVimToLua:test_keywords()
        before = strsplit('\n',
[[
function! VST_Headers(text)
let i = 0
    endfor
                endif
            endwhile
endfunction
    call insert(ltype, 'blank')
        if ltype[i] == 'blank'
            elseif g:ptype[i] == 'uli'
    for par in g:paras
    while i < len(g:paras)
]] )
        after = strsplit('\n', 
[[
function VST_Headers(text)
i = 0
    end
                end
            end
end
    table.insert(ltype, 0, 'blank')
        if ltype[i] == 'blank' then
            elseif ptype[i] == 'uli' then
    for par in paras do
    while i < len(paras) do
]] )
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_concatenation()
        before = strsplit('\n',
[[
            a.f().b
            let img = '{'.substitute(img, '^\s*,', '', 'ge').'}'
        a.b.c
        let aname = '<vim:a name="l'.stitle.'"></vim:a>\n'
]] )
        --substitute(g:paras[i], '$', '\n'.repeat(' ', g:pindent2).'</vim:p>\n', 'e')
        after = strsplit('\n', 
[[
            a..f()..b
            img = '{'..substitute(img, '^\s*,', '', 'ge')..'}'
        a..b..c
        aname = '<vim:a name="l'..stitle..'"></vim:a>\n'
]] )
        --substitute(paras[i], '$', '\n'.._repeat(' ', pindent2)..'</vim:p>\n', 'e')
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_comments()
        before = strsplit('\n',
[[
                " Region embraced , now I have to take care about paragraph
]] )
        after = strsplit('\n', 
[[
                -- Region embraced , now I have to take care about paragraph
]] )
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_variables()
        before = strsplit('\n',
[[
                call insert(g:plinen, 0, i)
            let b:toto = 0
            let w:toto = 0
            let s:toto = 0
            let l:toto = 0
    let doc = a:text
]] )
        after = strsplit('\n', 
[[
                table.insert(plinen, 0, i)
            toto = 0
            toto = 0
            toto = 0
            toto = 0
    doc = text
]] )
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_option()
        before = strsplit('\n',
    -- if &compatible == 1
[[
        set nocompatible
        set compatible
]] )
        after = strsplit('\n', 
    -- if getOption('compatible') == 1 then
[[
        setOption('compatible', false )
        setOption('compatible', true )
]] )
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_operators()
        before = strsplit('\n',
[[
                paras[parcounter] .= "\n".doc[i]
            if has_key(g:hlinkdb, title) && g:hlinkdb[title] != ''
                elseif attr == 'identify' || attr == 'target'
        if parline =~ '^ameint='
        if parline !~ '^ameint='
]] )
        after = strsplit('\n', 
[[
                paras[parcounter] = paras[parcounter].."\n"..doc[i]
            if has_key(hlinkdb, title) and hlinkdb[title] ~= '' then
                elseif attr == 'identify' or attr == 'target' then
        if VimRegexp('^ameint='):match(parline) then
        if not VimRegexp('^ameint='):match(parline) then
]] )
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_inc()
        before = strsplit('\n',
[[
        let j += 1
]] )
        after = strsplit('\n', 
[[
        j = j + 1
]] )
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_array()
        before = strsplit('\n',
[[
    let ltype = []
            let ltype += ['blank']
]] )
        after = strsplit('\n', 
[[
    ltype = {}
            table.insert( ltype, 'blank' )
]] )
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_conversion()
        before = strsplit('\n',
[[
                if plines[-1] =~ '^\s*\(\d\+\|[a-zA-Z]\)[\]:.)}]\s*\ze'
                let g:vst_headers[g:ptype[i] ] = repeat(lchar, 10)
]] )
        after = strsplit('\n', 
[[
                if VimRegexp('^\s*\(\d\+\|[a-zA-Z]\)[\]:.)}]\s*\ze'):match(plines[-1]) then
                vst_headers[ptype[i] ] = _repeat(lchar, 10)
]] )
        self:assertProcessStringList( before, after )
    end

    function TestVimToLua:test_unsupported()
        before = strsplit('\n',
[[
        exe line3
        %s/lt/>/ge
    z_rez = @z
            unlet! g:image
]] )
        after = strsplit('\n', 
[[
        -- XXX exe line3
        -- XXX %s/lt/>/ge
    -- XXX z_rez = @z
            -- XXX unlet! image
]] )
        self:assertProcessStringList( before, after )
    end


    function TestVimToLua:test_function()
        before = strsplit('\n',
[[
    strlen(toto)
    matchstr(s, sub)
    add(ltype, 'blank')
    insert(lindent, '')
]] )
        after = strsplit('\n', 
[[
    string.len(toto)
    VimRegexp(sub):matchResult(s)
    table.insert(ltype, 'blank')
    table.insert(lindent, 0, '')
]] )
        self:assertProcessStringList( before, after )
    end



if not _REQUIREDNAME then
    -- ret = LuaUnit:run('TestVimToLua:test_operators') -- will execute only one test
    ret = LuaUnit:run() -- will execute all tests
    setLuaReturnValue( ret )
end

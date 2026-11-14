-- snippets.lua
-- The examples of the manual are not copied into the chapters: a fenced
-- block whose only line is  --8<-- "examples/NAME.cc"  is replaced by the
-- file, as pymdownx.snippets does for the web site. The path is relative
-- to the root of the manual; a file that does not exist stops the build.

local root = '..'

local function read(path)
  local f = io.open(root .. '/' .. path, 'r')
  if not f then
    error('snippets.lua: cannot open ' .. path)
  end
  local s = f:read('a')
  f:close()
  return (s:gsub('\n$', ''))
end

function CodeBlock(el)
  local path = el.text:match('^%s*%-%-8<%-%-%s+"([^"]+)"%s*$')
  if path then
    el.text = read(path)
  end
  return el
end

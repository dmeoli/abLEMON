-- source-links.lua
-- The name of a file or folder of the repository written as code, e.g.,
-- `lemon/dijkstra.h`, becomes a link to it on GitHub, as hooks.py does on
-- the web site. Run before break-code.lua, which leaves the link alone.

local REPO = 'https://github.com/dmeoli/abLEMON'
local here = PANDOC_SCRIPT_FILE:match('(.*/)') or './'
local ROOT = here .. '../../'

local function kind(path)
  if not path:find('/') or path:find('%.%.') or
     not path:match('^[%w_.][%w_./%-]*$') then
    return nil
  end
  local f = io.open(ROOT .. path, 'r')
  if f then
    local ok = f:read(0)
    f:close()
    if ok ~= nil then
      return 'blob'
    end
  end
  local isdir = pcall(pandoc.system.list_directory, ROOT .. path)
  if isdir then
    return 'tree'
  end
  return nil
end

local function code(el)
  local k = kind(el.text)
  if not k then
    return nil
  end
  local path = el.text:gsub('/$', '')
  -- false: the code span within the new link is not visited again
  return pandoc.Link({ el }, REPO .. '/' .. k .. '/master/' .. path), false
end

-- top-down, so that a code span already within a link stays as it is
return {
  {
    traverse = 'topdown',
    Link = function(el) return el, false end,
    Code = code,
  }
}

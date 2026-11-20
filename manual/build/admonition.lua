-- admonition.lua
-- The notes of the manual are written for MkDocs as admonitions,
--     !!! note "abLEMON"
--         text indented by four spaces
-- which Pandoc reads as one paragraph starting with the tokens of the first
-- line. For the PDF, such a paragraph becomes the environment ablnote of
-- preamble.tex, with the title of the note as a run-in label. (The notes
-- of the manual are single paragraphs; a longer one would need a Div.)

function Para(el)
  if not FORMAT:match('latex') then
    return nil
  end
  local c = el.content
  if #c < 4 or c[1].t ~= 'Str' or c[1].text ~= '!!!' then
    return nil
  end
  -- skip "!!!", a space, the kind of note, a space, and the quoted title
  local i, title = 2, nil
  while i <= #c and title == nil do
    local x = c[i]
    if x.t == 'Quoted' then
      title = pandoc.utils.stringify(x.content)
    elseif x.t == 'Str' and x.text:match('^"') then
      title = x.text:gsub('"', '')
    end
    i = i + 1
  end
  if title == nil then
    return nil
  end
  while i <= #c and (c[i].t == 'Space' or c[i].t == 'SoftBreak') do
    i = i + 1
  end
  local body = { pandoc.RawInline('latex', '\\ablnotelabel{' .. title .. '}') }
  for k = i, #c do
    body[#body + 1] = c[k]
  end
  return {
    pandoc.RawBlock('latex', '\\begin{ablnote}'),
    pandoc.Para(body),
    pandoc.RawBlock('latex', '\\end{ablnote}')
  }
end

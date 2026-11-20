-- crossref-link.lua
-- Single-source cross-reference handling. In the Markdown source, an
-- internal reference is written as an ordinary relative link to another
-- chapter file plus an explicit anchor, e.g. `[Chapter 7](07-physical-
-- abstract.md#ch-7)` or `[§7.6](07-physical-abstract.md#sec-7-6)`. MkDocs
-- serves these natively (one HTML page per source file, anchors from the
-- explicit `{#id}` heading attributes). For the LaTeX (PDF) build all the
-- sources are concatenated into a single document, so the file part of the
-- target is meaningless. Doxygen (https://…) and figure links are untouched.
--
-- Why \hyperref and not a plain `#anchor` link: Pandoc renders an internal
-- `[text](#id)` link as `\hyperlink{id}{text}`, whose destination is the
-- `\hypertarget{id}{…}` it wraps around the heading. For a `\chapter`, that
-- hypertarget sits *before* the `\clearpage` the chapter triggers, so the
-- destination lands at the bottom of the previous (often half-empty) page.
-- Pandoc also emits a `\label{id}` *after* the sectioning command; that
-- label's anchor is the heading's real page anchor (the same one the auto
-- table of contents uses). So we emit `\hyperref[id]{text}` instead, which
-- targets the label and lands exactly on the heading's page. (Subsection
-- links are unaffected, since there is no page break, but stay correct under
-- \hyperref.) The labelled items (bibliography entries, algorithms,
-- exercises, ...) are <span id> elements, for which Pandoc writes a
-- \hypertarget and no \label: their links are \hyperlink.

function Link(el)
  if FORMAT:match('latex') or FORMAT:match('beamer') then
    local anchor = el.target:match('^[%w%./%-]+%.md#(.+)$')
    if anchor then
      -- a heading and a figure have a \label, an item (a <span id>) only a
      -- \hypertarget
      local cmd = '\\hyperlink{' .. anchor .. '}{'
      if anchor:match('^ch%-') or anchor:match('^sec%-') or
         anchor:match('^app%-') or anchor:match('^fig%-') then
        cmd = '\\hyperref[' .. anchor .. ']{'
      end
      local out = { pandoc.RawInline('latex', cmd) }
      for _, inl in ipairs(el.content) do out[#out + 1] = inl end
      out[#out + 1] = pandoc.RawInline('latex', '}')
      return out
    end
  end
  return el
end

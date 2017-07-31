local gossip = require('lua_bridge/gossip')
local ui = require('lua_bridge/ui')

local Widget = {}
Widget.__index = Widget

setmetatable(Widget, {
  __call = function (cls, ...)
    local self = setmetatable({}, Widget)
    self:create(...)
    return self
  end,
})

function Widget:create(initial_data)
  self.parent = nil

  self.data = initial_data
  self.scroll_y = 1.0
  self.max_scroll_y = #self.data
  self.chosen = 0
  self.dirty = true

  return self
end

function Widget:destroy()
end

function Widget:attach(parent)
  self.parent = parent

  self.parent:scroll_y(0)
  self.window_height = 16

  parent:on("click", function (e)
    self.chosen = self.scroll_y + e.y + 1
    self.dirty = true
  end)
  parent:on("scroll", function (e)
    local y = self.scroll_y - e.scroll_y
    if y >= 1 and y + (self.window_height - 1) < self.max_scroll_y then
      self.scroll_y = y

      self.parent:scroll_y(((y - 1) / (self.max_scroll_y - self.window_height - 1)) * 100)
      self.dirty = true
    end
  end)
end

function Widget:paint()
  if not self.dirty then
    return
  end
  self.dirty = false

  local data_end = self.scroll_y + self.window_height
  if data_end > #self.data then
    data_end = #self.data
  end

  self.parent:clear(string.byte(" "), 0, 0)
  for y = self.scroll_y, data_end do
    local str = self.data[y]

    local back_color = 0
    if self.chosen == y then
      back_color = 0x666666
    end
    for x = 1, 17 do
      local c = string.byte(" ")
      if x <= #str then
        c = string.byte(str:sub(x, x))
      end
      self.parent:glyph(c, x - 1, y - self.scroll_y - 1, 0xffffff, back_color)
    end
  end
end

return Widget

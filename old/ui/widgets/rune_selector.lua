local gossip = require('lua_bridge/gossip')

local Widget = {}
Widget.__index = Widget

setmetatable(Widget, {
  __call = function (cls, ...)
    local self = setmetatable({}, Widget)
    self:create(...)
    return self
  end,
})

function Widget:create()
  self.parent = nil

  self.chosen_rune = 1

  return self
end

function Widget:destroy()
end

function Widget:attach(parent)
  self.parent = parent

  parent:on("click", function (e)
    self.chosen_rune = (e.y * 16) + e.x
    gossip.emit("widget:rune_selected", self.chosen_rune)
  end)
end

function Widget:paint()
  for y = 0, 15 do
    for x = 0, 15 do
      local rune = (y * 16) + x
      local fore = 0x808080
      local back = 0x0

      if math.floor(rune / 16) == math.floor(self.chosen_rune / 16)
        or math.floor(rune % 16) == math.floor(self.chosen_rune % 16) then
        fore = 0xc8c8c8
        back = 0x666666
      end
      if rune == self.chosen_rune then
        fore = 0xffffff
        back = 0x999999
      end

      self.parent:glyph(rune, x, y, fore, back)
    end
  end
end

return Widget

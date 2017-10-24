local lua_bridge = require("lua_bridge")

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
  self.chosen_rune = 1

  self.events = {
    ["created"] = function (e)
      lua_bridge.emit_message(MSG_WORLD_EDIT_RUNE_SELECTED, {
        ["rune"] = self.chosen_rune,
      })
    end,

    ["click"] = function (e)
      self.chosen_rune = (e.y * 16) + e.x

      lua_bridge.emit_message(MSG_WORLD_EDIT_RUNE_SELECTED, {
        ["rune"] = self.chosen_rune,
      })
    end,
  }

  return self
end

function Widget:destroy()
end

function Widget:paint(canvas)
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

      canvas:glyph(rune, x, y, fore, back)
    end
  end
end

function Widget:trigger(id, data)
  if self.events[id] ~= nil then
    self.events[id](data)
  end
end

return Widget

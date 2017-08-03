local lua_bridge = require("lua_bridge")
local ui = require("ui")

local Window = {}
Window.__index = Window

setmetatable(Window, {
  __call = function (cls, ...)
    local self = setmetatable({}, Window)
    self:create(...)
    return self
  end,
})

function Window:create(title, x, y, width, height)
  self.chosen_rune = 1

  self.events = {
    ["created"] = function (e)
      self:paint()
    end,

    ["mousemove"] = function (e)
      io.write("received mousemove\n")
    end,

    ["click"] = function (e)
      self.chosen_rune = (e.y * 16) + e.x
      self:paint()

      lua_bridge.emit_message(MSG_WORLD_EDIT_RUNE_SELECTED, {
        ["rune"] = self.chosen_rune,
      })
    end,

    ["scroll"] = function (e)
      io.write("received scroll\n")
    end,
  }

  ui.window_create(self, title, x, y, width, height)

  return self
end

function Window:destroy()
  ui.window_destroy(self)
end

function Window:paint()
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

      self:glyph(rune, x, y, fore, back)
    end
  end
end

function Window:glyph(rune, x, y, fore_color, back_color)
  ui.window_glyph(self, rune, x, y, fore_color, back_color)
end

function Window:trigger(id, data)
  if self.events[id] ~= nil then
    self.events[id](data)
  end
end

return Window

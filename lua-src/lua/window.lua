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
  self.widget = nil

  self.events = {
    ["created"] = function (e)
      self:paint()
    end,

    ["mousemove"] = function (e)
      io.write("received mousemove\n")
      self:paint()
    end,

    ["click"] = function (e)
      io.write("received click\n")
      self:paint()
    end,

    ["scroll"] = function (e)
      io.write("received scroll\n")
      self:paint()
    end,
  }

  ui.window_create(self, title, x, y, width, height)

  return self
end

function Window:destroy()
  ui.window_destroy(self)
end

function Window:widget(widget)
  self.widget = widget
end

function Window:paint()
  if self.widget ~= nil then
    self.widget:paint(self)
  end
end

function Window:glyph(rune, x, y, fore_color, back_color)
  ui.window_glyph(self, rune, x, y, fore_color, back_color)
end

function Window:trigger(id, data)
  if self.widget ~= nil then
    self.widget:trigger(id, data)
  end

  if self.events[id] ~= nil then
    self.events[id](data)
  end
end

return Window

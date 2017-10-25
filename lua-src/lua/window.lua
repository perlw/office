local lua_bridge = require("lua_bridge")
local ui = require("ui")
local new_object = require("new_object")

local Canvas = new_object()
function Canvas:create(width, height)
  self.width = width
  self.height = height
  self.glyphs = {}
  self.dirty = false
end

function Canvas:glyph(rune, x, y, fore_color, back_color)
  local index = (y * self.width) + x
  if self.glyphs[index] == nil then
    self.dirty = true
  else
    if self.glyphs[index].rune ~= rune or
      self.glyphs[index].fore_color ~= fore_color or
      self.glyphs[index].back_color ~= back_color then
      self.dirty = true
    end
  end

  self.glyphs[index] = {
    rune = rune,
    x = x,
    y = y,
    fore_color = fore_color,
    back_color = back_color,
  }
end

local Window = new_object()

function Window:create(title, x, y, width, height)
  self.widget = nil
  self.canvas = Canvas(width, height)

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
  if type(self.widget) == "table" then
    self.widget:paint(self.canvas)
    if self.canvas.dirty then
      self.canvas.dirty = false
      ui.window_glyphs(self, self.canvas.glyphs)
    end
  end
end

function Window:trigger(id, data)
  if type(self.widget) == "table" then
    self.widget:trigger(id, data)
  end

  if self.events[id] ~= nil then
    self.events[id](data)
  end
end

return Window

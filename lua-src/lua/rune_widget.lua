local Widget = {}
Widget.__index = Widget

setmetatable(Widget, {
  __call = function (cls, ...)
    local self = setmetatable({}, Widget)
    self:create(...)
    return self
  end,
})

function Widget:create(initial_rune)
  self.chosen_rune = initial_rune
  self.listeners = {}

  self.events = {
    ["click"] = function (e)
      self.chosen_rune = (e.y * 16) + e.x
      self:emit("selected", self.chosen_rune)
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

function Widget:emit(id, data)
  if self.listeners[id] ~= nil then
    for _, callback in ipairs(self.listeners[id]) do
      callback(data)
    end
  end
end

function Widget:on(id, callback)
  if self.listeners[id] == nil then
    self.listeners[id] = {}
  end
  self.listeners[id][#self.listeners[id] + 1] = callback
end

function Widget:rune()
  return self.chosen_rune
end

return Widget
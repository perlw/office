function new_object(clone_object)
  local Object = clone_object or {}
  Object.__index = Object

  setmetatable(Object, {
    __call = function (cls, ...)
      local self = setmetatable({}, Object)
      self:create(...)
      return self
    end,
  })

  return Object
end

------------------------
local Events = {
  listeners = {}
}

function Events:emit(id, data)
  if self.listeners[id] ~= nil then
    for _, callback in ipairs(self.listeners[id]) do
      callback(data)
    end
  end
end

function Events:on(id, callback)
  if self.listeners[id] == nil then
    self.listeners[id] = {}
  end
  self.listeners[id][#self.listeners[id] + 1] = callback
end
------------------------

local Widget = new_object(Events)

function hue_to_rgb(p, q, t)
  if t < 0 then
    t = t + 1
  end
  if t > 1 then
    t = t - 1
  end
  if t < 1/6 then
    return p + (q - p) * 6 * t
  elseif t < 1/2 then
    return q
  elseif t < 2/3 then
    return p + (q - p) * (2/3 - t) * 6
  end
  return p
end

function hsl_to_rgb(h, s, l)
  local r = l
  local g = l
  local b = l

  if s > 0.0 then
    local q = 0
    if l < 0.5 then
      q = l * (1 + s)
    else
      q = l + s - l * s
    end
    local p = 2 * l - q
    r = hue_to_rgb(p, q, h + 1/3)
    g = hue_to_rgb(p, q, h)
    b = hue_to_rgb(p, q, h - 1/3)
  end

  r = math.floor((r * 255) + 0.5)
  g = math.floor((g * 255) + 0.5)
  b = math.floor((b * 255) + 0.5)

  return (r << 16) + (g << 8) + b
end

function Widget:create(initial_index)
  self.chosen_color = initial_index

  self.events = {
    ["click"] = function (e)
      self.chosen_color = (e.y * 16) + e.x
      self:emit("selected", self:color())
    end,
  }

  return self
end

function Widget:destroy()
end

function Widget:paint(canvas)
  for y = 0, 15 do
    for x = 0, 15 do
      local rune = 219
      local fore = hsl_to_rgb(y / 16, 1.0, x / 16)
      if (y * 16) + x == self.chosen_color then
        rune = "*"
      end

      canvas:glyph(rune, x, y, fore, 0x0)
    end
  end
end

function Widget:trigger(id, data)
  if self.events[id] ~= nil then
    self.events[id](data)
  end
end

function Widget:color()
  return hsl_to_rgb(math.floor(self.chosen_color / 16) / 16, 1.0, math.floor(self.chosen_color % 16) / 16)
end

return Widget

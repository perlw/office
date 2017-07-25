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

function Widget:create()
  self.parent = nil

  self.chosen_color = 1
  self.color_rune = 1

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

  self.gossip_widget_handle = gossip.subscribe("widget:rune_selected", function (group_id, id, rune)
    if self.widget_events[id] ~= nil then
      if rune == nil then
        return
      end

      self.color_rune = rune
    end
  end)

  return self
end

function Widget:destroy()
  gossip.unsubscribe(self.gossip_widget_handle)
end

function Widget:attach(parent)
  self.parent = parent

  parent:on("click", function (e)
    self.chosen_color = (e.y * 16) + e.x
    local fore = hsl_to_rgb(e.y / 16, 1.0, e.x / 16);
    gossip.emit("widget:color_selected", fore)
  end)
end

function Widget:paint()
  for y = 0, 15 do
    for x = 0, 15 do
      local rune = self.color_rune
      local fore = hsl_to_rgb(y / 16, 1.0, x / 16);
      if (y * 16) + x == self.chosen_color then
        rune = string.byte("*")
      end

      self.parent:glyph(rune, x, y, fore, 0x0)
    end
  end
end

return Widget

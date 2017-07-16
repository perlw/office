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

function Widget:create(window)
  assert(window)

  self.window = window
  if self.window.handle == nil then
    io.write("UI:RuneSelector> failed to attach to window...\n")
  end

  self.chosen_rune = 1

  self.window_events = {
    ["mousemove"] = function (e) end,
    ["click"] = function (e)
      self.chosen_rune = (e.y * 16) + e.x
      gossip.emit("widget:rune_selected", self.chosen_rune)
    end,
  }
  self.gossip_window_handle = gossip.subscribe("window:*", function (group_id, id, e)
    if id ~= "mousemove" and id ~= "click" then
      return
    end

    if e.target ~= self.window.handle then
      return
    end

    self.window_events[id](e)
  end)

  self.gossip_widget_handle = gossip.subscribe("widget:paint", function (group_id, id, e)
    if e ~= nil and e.target ~= self.window.handle then
      return
    end

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

        self.window:glyph(rune, x, y, fore, back)
      end
    end
  end)

  self.window:on("close", function ()
    self:destroy()
  end)

  return self
end

function Widget:destroy()
  gossip.unsubscribe(self.gossip_widget_handle)
  gossip.unsubscribe(self.gossip_window_handle)
end

return Widget

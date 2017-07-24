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
    io.write("UI:List> failed to attach to window...\n")
  end

  self.chosen_rune = 1

  self.window_events = {
    ["mousemove"] = function (e) end,
    ["click"] = function (e) end,
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

local gossip = require('lua_bridge/gossip')
local ui = require('lua_bridge/ui')

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
  self.handle = ui.window_create(title, x, y, width, height)
  if self.handle == nil then
    io.write("UI:Window> failed to create window...\n")
    self.handle = 0
  end

  self.window_events = {
    ["mousemove"] = function (e) end,
    ["click"] = function (e) end,
    ["close"] = function () end,
  }
  self.gossip_window_handle = gossip.subscribe("window:*", function (group_id, id, e)
    if id ~= "mousemove" and id ~= "click" then
      return
    end

    if e.target ~= self.handle then
      return
    end

    self.window_events[id](e)
  end)

  return self
end

function Window:destroy()
  self.window_events["close"]()

  gossip.unsubscribe(self.gossip_window_handle)

  ui.window_destroy(self.handle)
end

function Window:glyph(rune, x, y, fore_color, back_color)
  ui.window_glyph(self.handle, rune, x, y, fore_color, back_color)
end

function Window:on(event, callback)
  self.window_events[event] = callback
end

return Window

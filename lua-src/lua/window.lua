local gossip = require('gossip')
local ui = require('ui')

local Window = {}
Window.__index = Window

setmetatable(Window, {
  __call = function (cls, ...)
    local self = setmetatable({}, Window)
    self:create(...)
    return self
  end,
})

function Window:create(x, y, width, height)
  self.handle = ui.window_create("LUA FontSel", x, y, width, height)
  if self.handle == nil then
    io.write("UI:Window> failed to create window...\n")
    self.handle = 0
  end
  self.click_callback = function()end

  local gossip_events = {
    ["mousemove"] = function (e)
    end,
    ["click"] = function (e)
      self.click_callback(e)
    end,
  }

  self.gossip_handle = gossip.subscribe("window:*", function (id, e)
    if e.target ~= self.handle then
      return
    end

    gossip_events[id](e)
  end)

  return self
end

function Window:destroy()
  gossip.unsubscribe(self.click_handle)

  ui.window_destroy(self.handle)
end

function Window:glyph(rune, x, y, fore_color, back_color)
  ui.window_glyph(self.handle, rune, x, y, fore_color, back_color)
end

function Window:on_click(callback)
  self.click_callback = callback
end

return Window

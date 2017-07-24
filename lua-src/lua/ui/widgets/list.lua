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

function Widget:create(window, initial_data)
  assert(window)

  self.window = window
  if self.window.handle == nil then
    io.write("UI:List> failed to attach to window...\n")
  end

  self.data = initial_data
  self.scroll_y = 1.0
  self.max_scroll_y = #self.data
  self.chosen = 0
  self.dirty = true

  self.window:scroll_y(0)
  self.window_height = 16

  self.window_events = {
    ["mousemove"] = function (e) end,
    ["click"] = function (e)
      self.chosen = self.scroll_y + e.y + 1
      self.dirty = true
    end,
    ["scroll"] = function (e)
      local y = self.scroll_y - e.scroll_y
      if y >= 1 and y + (self.window_height - 1) < self.max_scroll_y then
        self.scroll_y = y

        self.window:scroll_y(((y - 1) / (self.max_scroll_y - self.window_height - 1)) * 100)
        self.dirty = true
      end
    end,
  }
  self.gossip_window_handle = gossip.subscribe("window:*", function (group_id, id, e)
    if id ~= "mousemove" and id ~= "click" and id ~= "scroll" then
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

    if not self.dirty then
      return
    end
    self.dirty = false

    local data_end = self.scroll_y + self.window_height
    if data_end > #self.data then
      data_end = #self.data
    end

    self.window:clear(string.byte(" "), 0, 0)
    for y = self.scroll_y, data_end do
      local str = self.data[y]

      local back_color = 0
      if self.chosen == y then
        back_color = 0x666666
      end
      for x = 1, 17 do
        local c = string.byte(" ")
        if x <= #str then
          c = string.byte(str:sub(x, x))
        end
        self.window:glyph(c, x - 1, y - self.scroll_y - 1, 0xffffff, back_color)
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

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
  self.handle = nil
  self.widget = nil

  self.gossip_ui_handle = gossip.subscribe("ui:*", function (group_id, id, e)
    if self.handle == nil then
      if id == "window_created" then
        self.handle = e
        return
      end
    end
  end)

  self.window_events = {
    ["mousemove"] = function (e) end,
    ["click"] = function (e) end,
    ["scroll"] = function (e) end,
  }
  self.gossip_window_handle = gossip.subscribe("window:*", function (group_id, id, e)
    if e ~= nil and e.target ~= self.handle then
      return
    end

    if self.window_events[id] ~= nil then
      self.window_events[id](e)
    end
  end)

  self.gossip_widget_handle = gossip.subscribe("widget:paint", function (group_id, id, e)
    if e ~= nil and e.target ~= self.handle then
      return
    end

    if self.widget ~= nil and self.widget.paint ~= nil then
      self.widget:paint(self)
    end
  end)

  ui.window_create(title, x, y, width, height)

  return self
end

function Window:destroy()
  if self.widget ~= nil and self.widget.destroy ~= nil then
    self.widget:destroy()
  end

  gossip.unsubscribe(self.gossip_widget_handle)
  gossip.unsubscribe(self.gossip_window_handle)
  gossip.unsubscribe(self.gossip_ui_handle)

  ui.window_destroy(self.handle)
end

function Window:content(content)
  self.widget = content
  self.widget:attach(self)
end

function Window:clear(rune, fore_color, back_color)
  --ui.window_clear(self.handle, rune, fore_color, back_color)
end

function Window:glyph(rune, x, y, fore_color, back_color)
  ui.window_glyph(self.handle, rune, x, y, fore_color, back_color)
end

function Window:scroll_x(scroll)
  --ui.window_scroll_x(self.handle, scroll)
end

function Window:scroll_y(scroll)
  --ui.window_scroll_y(self.handle, scroll)
end

function Window:on(event, callback)
  self.window_events[event] = callback
end

return Window

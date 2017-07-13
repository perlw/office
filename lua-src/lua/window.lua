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
  self.handle = ui.window_create(x, y, width, height)
  return self
end

function Window:destroy()
  ui.window_destroy(self.handle)
end

return Window

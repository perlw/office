local Window = require("window")

local Scene = {}
Scene.__index = Scene

setmetatable(Scene, {
  __call = function (cls, ...)
    local self = setmetatable({}, Scene)
    self:create(...)
    return self
  end,
})

Scene.systems = {
  "ui",
}

function Scene:create()
  self.windows = {}
  self.windows[#windows + 1] = Window("Test Wnd 1", 141, 7, 18, 18)
  self.windows[#windows + 1] = Window("Test Wnd 2", 141, 26, 18, 18)

  return self
end

function Scene:destroy()
  for _, window in ipairs(self.windows) do
    window:destroy()
  end
end

function Scene:trigger(id, data)
  --[[if self.events[id] ~= nil then
    self.events[id](data)
  end]]--
end

return Scene

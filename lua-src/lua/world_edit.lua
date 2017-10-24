local Window = require("window")
local RuneWidget = require("rune_widget")
local ColorWidget = require("color_widget")

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
  self.rune_window = Window("RuneSel", 141, 7, 18, 18)
  self.color_window = Window("ColSel", 141, 26, 18, 18)

  self.rune_window:widget(RuneWidget())
  self.color_window:widget(ColorWidget())

  return self
end

function Scene:destroy()
  self.rune_window:destroy()
  self.color_window:destroy()
end

function Scene:trigger(id, data)
  --[[if self.events[id] ~= nil then
    self.events[id](data)
  end]]--
end

return Scene

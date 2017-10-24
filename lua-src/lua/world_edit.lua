local lua_bridge = require("lua_bridge")

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

  self.rune_widget = RuneWidget(1)
  self.color_widget = ColorWidget(15)

  lua_bridge.emit_message(MSG_WORLD_EDIT_RUNE_SELECTED, {
    ["rune"] = self.rune_widget:rune(),
  })
  self.rune_widget:on("selected", function (rune)
    lua_bridge.emit_message(MSG_WORLD_EDIT_RUNE_SELECTED, {
      ["rune"] = rune,
    })
  end)

  lua_bridge.emit_message(MSG_WORLD_EDIT_COLOR_SELECTED, {
    ["color"] = self.color_widget:color(),
  })
  self.color_widget:on("selected", function (color)
    lua_bridge.emit_message(MSG_WORLD_EDIT_COLOR_SELECTED, {
      ["color"] = color,
    })
  end)

  self.rune_window:widget(self.rune_widget)
  self.color_window:widget(self.color_widget)

  return self
end

function Scene:destroy()
  self.rune_window:destroy()
  self.color_window:destroy()
end

return Scene

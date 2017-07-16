local gossip = require("lua_bridge/gossip")
local Window = require("ui/window")
local RuneSelector = require("ui/widgets/rune_selector")
local ColorSelector = require("ui/widgets/color_selector")

action("close", function ()
  gossip.emit("game:kill")
end)
action("prev_scene", function ()
  gossip.emit("scene:prev")
end)
action("next_scene", function ()
  gossip.emit("scene:next")
end)

gossip.subscribe("game:init", function ()
  io.write("LUA: Welcome!\n")
end)

gossip.subscribe("game:kill", function ()
  io.write("LUA: Goodbye...\n")
end)

local rune_sel_window = nil
local rune_sel_widget = nil
local color_sel_window = nil
local color_sel_widget = nil
gossip.subscribe("scene:setup", function (group_id, id, scene)
  if scene == "world-edit" then
    rune_sel_window = Window("LUA RuneSel", 60, 20, 18, 18)
    rune_sel_widget = RuneSelector(rune_sel_window)
    color_sel_window = Window("ColorSel", 60, 40, 18, 18)
    color_sel_widget = ColorSelector(color_sel_window)
  end
end)

gossip.subscribe("scene:teardown", function (group_id, id, scene)
  if scene == "world-edit" then
    if rune_sel_window then
      rune_sel_window:destroy()

      rune_sel_widget = nil
      rune_sel_window = nil
    end
    if color_sel_window then
      color_sel_window:destroy()

      color_sel_widget = nil
      color_sel_window = nil
    end
  end
end)

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
action("player_move_up_left", function ()
  gossip.emit("player:move_up_left")
end)
action("player_move_left", function ()
  gossip.emit("player:move_left")
end)
action("player_move_up_right", function ()
  gossip.emit("player:move_up_right")
end)
action("player_move_right", function ()
  gossip.emit("player:move_right")
end)
action("player_move_up", function ()
  gossip.emit("player:move_up")
end)
action("player_move_down_left", function ()
  gossip.emit("player:move_down_left")
end)
action("player_move_down", function ()
  gossip.emit("player:move_down")
end)
action("player_move_down_right", function ()
  gossip.emit("player:move_down_right")
end)
action("pause_simulation", function ()
  gossip.emit("scene:pause_updates")
end)

gossip.subscribe("game:init", function ()
  io.write("LUA: Welcome!\n")
end)

gossip.subscribe("game:kill", function ()
  io.write("LUA: Goodbye...\n")
  teardown_world()
end)

local rune_sel_window = nil
local rune_sel_widget = nil
local color_sel_window = nil
local color_sel_widget = nil
function setup_world()
  rune_sel_window = Window("LUA RuneSel", 141, 7, 18, 18)
  rune_sel_widget = RuneSelector(rune_sel_window)
  color_sel_window = Window("ColorSel", 141, 26, 18, 18)
  color_sel_widget = ColorSelector(color_sel_window)
end

function teardown_world()
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

gossip.subscribe("scene:setup", function (group_id, id, scene)
  if scene == "world-edit" then
    setup_world()
  end
end)

gossip.subscribe("scene:teardown", function (group_id, id, scene)
  if scene == "world-edit" then
    teardown_world()
  end
end)

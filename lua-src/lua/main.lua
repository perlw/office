local gossip = require("lua_bridge/gossip")
local Window = require("ui/window")
local RuneSelector = require("ui/widgets/rune_selector")

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

local window = nil
local rune_sel = nil
gossip.subscribe("scene:setup", function (id, scene)
  if scene == "world-edit" then
    window = Window("LUA RuneSel", 60, 40, 18, 18)
    rune_sel = RuneSelector(window)
  end
end)

gossip.subscribe("scene:teardown", function (id, scene)
  if scene == "world-edit" then
    if window then
      window:destroy()

      rune_sel = nil
      window = nil
    end
  end
end)

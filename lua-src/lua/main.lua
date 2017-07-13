local gossip = require("gossip")
local testlib = require("testlib")
local Window = require("window")

action("close", function ()
  gossip.emit("game:kill")
end)
action("prev_scene", function ()
  gossip.emit("scene:prev")
end)
action("next_scene", function ()
  gossip.emit("scene:next")
end)

testlib.func1()
testlib.func2()
testlib.func1()

local window = nil

gossip.subscribe("game:init", function ()
  io.write("LUA: Welcome!\n")
end)

gossip.subscribe("game:kill", function ()
  io.write("LUA: Goodbye...\n")
end)

gossip.subscribe("scene:setup", function (scene)
  if scene == "world-edit" then
    window = Window(0, 0, 10, 10)
  end
end)

gossip.subscribe("scene:teardown", function (scene)
  if scene == "world-edit" then
    if window then
      window:destroy()
      window = nil
    end
  end
end)

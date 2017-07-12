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
  io.write("Welcome!\n")
end)

gossip.subscribe("game:kill", function ()
  io.write("Goodbye...\n")

  if window then
    window:destroy()
  end
end)

gossip.subscribe("scene:changed", function (scene)
  if scene == "world-edit" then
    window = Window(0, 0, 10, 10)
  else
    if window then
      window:destroy()
    end
  end
end)

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

  window = Window(0, 0, 10, 10)
end)

gossip.subscribe("game:kill", function ()
  io.write("Goodbye...\n")

  window:destroy()
end)

function update(dt)
  --io.write(dt .. "\n")
end

local gossip = require("gossip")
local testlib = require("testlib")

action("custom_close", function ()
  gossip.emit("game:kill")
end)

testlib.func1()
testlib.func2()
testlib.func1()

gossip.subscribe("game:kill", function ()
  io.write("Goodbye...\n")
end)

function update(dt)
  --io.write(dt .. "\n")
end

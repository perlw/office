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

local chosen_rune = 1;
gossip.subscribe("scene:setup", function (id, scene)
  if scene == "world-edit" then
    window = Window(60, 40, 18, 18)

    window:on_click(function (e)
      local rune = (e.y * 16) + e.x
      chosen_rune = rune
      gossip.emit("widget:rune_selected", rune)
    end)
  end
end)

gossip.subscribe("widget:paint", function (id, rune)
  for y = 0, 15 do
    for x = 0, 15 do
      local rune = (y * 16) + x
      local fore = 0x808080
      local back = 0x0

      if math.floor(rune / 16) == math.floor(chosen_rune / 16)
        or math.floor(rune % 16) == math.floor(chosen_rune % 16) then
        fore = 0xc8c8c8
        back = 0x666666
      end
      if rune == chosen_rune then
        fore = 0xffffff
        back = 0x999999
      end

      window:glyph(rune, x, y, fore, back)
    end
  end
end)

gossip.subscribe("scene:teardown", function (id, scene)
  if scene == "world-edit" then
    if window then
      window:destroy()
      window = nil
    end
  end
end)

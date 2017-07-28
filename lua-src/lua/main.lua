--local gossip = require("lua_bridge/gossip")
--[[local Window = require("ui/window")
local RuneSelector = require("ui/widgets/rune_selector")
local ColorSelector = require("ui/widgets/color_selector")
local List = require("ui/widgets/list")]]

--[[
gossip.subscribe("game:init", function ()
  io.write("LUA: Welcome!\n")
end)

gossip.subscribe("game:kill", function ()
  io.write("LUA: Goodbye...\n")
  teardown_world()
end)
]]

local windows = {}
function setup_world()
  --windows[#windows + 1] = Window("RuneSel", 141, 7, 18, 18)
  --windows[#windows]:content(RuneSelector())

--[[  windows[#windows + 1] = Window("ColorSel", 141, 26, 18, 18)
  windows[#windows]:content(ColorSelector())

  windows[#windows + 1] = Window("List", 141, 45, 18, 18)
  windows[#windows]:content(List({
    "foo",
    "bar",
    "c:\\foo\\bar",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
    "a",
    "b",
    "c",
    "d",
    "e",
    "f",
    "g",
    "h",
    "i",
    "j",
    "k",
    "l",
    "m",
    "n",
    "o",
    "p",
    "q",
    "r",
  }))
  ]]
end

function teardown_world()
  for _,window in pairs(windows) do
    window:destroy()
  end
  windows = {}
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

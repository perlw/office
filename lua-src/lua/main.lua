math.randomseed(os.time())

local lua_bridge = require("lua_bridge")
local load_tiledefs = require("tiledefs")

local windows = {}

local player_moves = {
  ["plr_move_dnlt"] = MSG_PLAYER_MOVE_DOWN_LEFT,
  ["plr_move_dn"] = MSG_PLAYER_MOVE_DOWN,
  ["plr_move_dnrt"] = MSG_PLAYER_MOVE_DOWN_RIGHT,
  ["plr_move_lt"] = MSG_PLAYER_MOVE_LEFT,
  ["plr_move_rt"] = MSG_PLAYER_MOVE_RIGHT,
  ["plr_move_uplt"] = MSG_PLAYER_MOVE_UP_LEFT,
  ["plr_move_up"] = MSG_PLAYER_MOVE_UP,
  ["plr_move_uprt"] = MSG_PLAYER_MOVE_UP_RIGHT,
}
local events = {
  [MSG_INPUT_KEY] = function (data)
    io.write("Lua: Got key\n")
    for key, val in pairs(data) do
      io.write("\t" .. key .. " -> " .. tostring(val) .. "\n")
    end
  end,

  [MSG_INPUT_ACTION] = function (data)
    io.write("Lua: Got action: " .. data.action .. "\n")

    if data.pressed then
      if player_moves[data.action] ~= nil then
        lua_bridge.emit_message(player_moves[data.action], nil)
      end
    end
  end,

  [MSG_MATERIALS_LOAD] = function (data)
    load_tiledefs()
  end,

  [MSG_SCENE_SETUP] = function (data)
    if data.scene == "scene_world-edit" then
      lua_bridge.post_message("systems", MSG_SYSTEM_START, {
        ["system"] = "ui",
      })
    end
  end,
  [MSG_SCENE_TEARDOWN] = function (data)
    if data.scene == "scene_world-edit" then
      lua_bridge.post_message("systems", MSG_SYSTEM_STOP, {
        ["system"] = "ui",
      })
    end
  end,

  [MSG_SYSTEM_SPUN_UP] = function (data)
    if data.system == "ui" then
      windows[#windows + 1] = os.time() + math.random()
      lua_bridge.post_message("ui", MSG_UI_WINDOW_CREATE, {
        ["title"] = "Test window",
        ["x"] = 141,
        ["y"] = 7,
        ["width"] = 18,
        ["height"] = 18,
        ["window_id"] = windows[#windows],
      })
    end
  end,
  [MSG_SYSTEM_SHUT_DOWN] = function (data)
    if data.system == "ui" then
      windows = {}
    end
  end,

  [MSG_UI_WINDOW_CREATED] = function (data)
    for _, id in ipairs(windows) do
      if data.window_id == id then
        io.write("found window\n")
      end
    end
  end,
}
lua_bridge.on_message(function (msg, data)
  if events[msg] ~= nil then
    events[msg](data)
  end
end)

lua_bridge.post_message("lua_bridge", MSG_DEBUG_TEST, nil)
lua_bridge.post_message("lua_bridge", MSG_DEBUG_TEST, {
  ["byte"] = "a",
  ["bool"] = true,
  ["uint"] = 1337,
  ["int"] = -42,
  ["float"] = 133.7,
  ["double"] = 1.3333333333333333333333337,
  ["string"] = "foobar",
  ["ptr"] = 0xdeadbeef,
})
lua_bridge.emit_message(MSG_DEBUG_TEST, {
  ["foo"] = "bar",
})

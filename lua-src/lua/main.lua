local lua_bridge = require("lua_bridge")

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
    require("tiledefs")
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

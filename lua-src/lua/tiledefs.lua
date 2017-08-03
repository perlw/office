local lua_bridge = require("lua_bridge")

lua_bridge.post_message("scene_game", MSG_MATERIAL_REGISTER, {
  ["id"] = "rock",
  ["rune"] = 178,
  ["fore_color"] = 0x999999,
  ["back_color"] = 0x0,
  ["tags"] = { "wall" },
})
lua_bridge.post_message("scene_game", MSG_MATERIAL_REGISTER, {
  ["id"] = "dirt",
  ["rune"] = 46,
  ["fore_color"] = 0x662f00,
  ["back_color"] = 0x0,
  ["tags"] = { "ground" },
})
lua_bridge.post_message("scene_game", MSG_MATERIAL_REGISTER, {
  ["id"] = "water",
  ["rune"] = 247,
  ["fore_color"] = 0x0047ab,
  ["back_color"] = 0x0,
  ["tags"] = { "fluid" },
})

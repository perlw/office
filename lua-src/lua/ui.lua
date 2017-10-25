local lua_bridge = require("lua_bridge")

local module = {}
windows = {}

function trigger_windows(event, data)
  for _, window in ipairs(windows) do
    if window.handle == data.handle then
      window:trigger(event, data)
    end
  end
end

module.events = {
  [MSG_UI_WINDOW_CREATED] = function (data)
    trigger_windows("created", data)
  end,

  [MSG_UI_WINDOW_MOUSEMOVE] = function (data)
    trigger_windows("mousemove", data)
  end,

  [MSG_UI_WINDOW_CLICK] = function (data)
    trigger_windows("click", data)
  end,

  [MSG_UI_WINDOW_SCROLL] = function (data)
    trigger_windows("scroll", data)
  end,
}

function module.window_create(window, title, x, y, width, height)
  window.handle = os.time() + math.random(1000000)
  windows[#windows + 1] = window
  lua_bridge.post_message("ui", MSG_UI_WINDOW_CREATE, {
    ["title"] = title,
    ["x"] = x,
    ["y"] = y,
    ["width"] = width,
    ["height"] = height,
    ["handle"] = window.handle,
  })
end

function module.window_destroy(window)
  lua_bridge.post_message("ui", MSG_UI_WINDOW_DESTROY, {
    ["handle"] = window.handle,
  })
end

function module.window_glyph(window, rune, x, y, fore_color, back_color)
  lua_bridge.post_message("ui", MSG_UI_WINDOW_GLYPH, {
    ["handle"] = window.handle,
    ["rune"] = rune,
    ["x"] = x,
    ["y"] = y,
    ["fore_color"] = fore_color,
    ["back_color"] = back_color,
  })
end

function module.window_glyphs(window, glyphs)
  lua_bridge.post_message("ui", MSG_UI_WINDOW_GLYPHS, {
    ["handle"] = window.handle,
    ["glyphs"] = glyphs,
  })
end

function module.handle_message(msg, data)
  if module.events[msg] ~= nil then
    module.events[msg](data)
  end
end

return module

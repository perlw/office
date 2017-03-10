resolution(640, 480)
gl_debug(true)

-- TODO: Add support to create complex combinations
-- Callbacks?
bind(INPUT_ACTION_CLOSE, KEY_ESCAPE)
bind(INPUT_ACTION_TEST, KEY_SPACE)

--[[
action(INPUT_ACTION_TEST, function ()
  io.write("foo")
end)
]]--

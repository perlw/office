resolution(640, 480)
gl_debug(true)

-- TODO: Add support to create complex combinations
-- Callbacks?
bind("close", KEY_ESCAPE)
bind("test", KEY_SPACE)

--[[
action(INPUT_ACTION_TEST, function ()
  io.write("foo")
end)
]]--

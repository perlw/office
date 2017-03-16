action("test", function ()
  io.write("test called from lua\n")
end)

action("foo", function ()
  io.write("FOOBAR\n")
end)

function update(dt)
  --io.write(dt .. "\n")
end

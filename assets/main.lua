action("test", function ()
  io.write("test called from lua\n")
end)

action("foo", function ()
  io.write("FOOBAR\n")
end)

function make_leet(arg)
  return (1337 - arg)
end

function update(dt)
  --io.write(dt .. "\n")
end

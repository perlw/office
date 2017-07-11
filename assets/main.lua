action("custom_close", function ()
  io.write("EEK\n")
end)

testlib.func1()
testlib.func2()
testlib.func1()

function update(dt)
  --io.write(dt .. "\n")
end

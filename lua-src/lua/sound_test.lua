local new_object = require("new_object")

local Scene = new_object()

Scene.systems = {
  "ui",
}

function Scene:create()
  return self
end

function Scene:destroy()
end

return Scene

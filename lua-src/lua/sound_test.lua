local Scene = {}
Scene.__index = Scene

setmetatable(Scene, {
  __call = function (cls, ...)
    local self = setmetatable({}, Scene)
    self:create(...)
    return self
  end,
})

Scene.systems = {
  "ui",
}

function Scene:create()
  return self
end

function Scene:destroy()
end

return Scene

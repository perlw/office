local Mixin = {
  listeners = {},
}

function Mixin:emit(id, data)
  if self.listeners[id] ~= nil then
    for _, callback in ipairs(self.listeners[id]) do
      callback(data)
    end
  end
end

function Mixin:on(id, callback)
  if self.listeners[id] == nil then
    self.listeners[id] = {}
  end
  self.listeners[id][#self.listeners[id] + 1] = callback
end

return Mixin

function new_object(mixins)
  local Object = {}
  Object.__index = Object

  if type(mixins) == "table" then
    for _, mixin in ipairs(mixins) do
      for k,v in pairs(mixin) do
        Object[k] = v
      end
    end
  end

  setmetatable(Object, {
    __call = function (cls, ...)
      local self = setmetatable({}, Object)
      self:create(...)
      return self
    end,
  })

  return Object
end

return new_object

local lua_bridge = require("lua_bridge")

local module = {}

module.current_scene = nil
module.scene_instance = nil
module.systems = {}
module.scenes = {}
module.events = {
  [MSG_SCENE_SETUP] = function (data)
    if module.scenes[data.scene] ~= nil then
      module.current_scene = module.scenes[data.scene]
      for _, system in ipairs(module.current_scene.systems) do
        lua_bridge.post_message("systems", MSG_SYSTEM_START, {
          ["system"] = system,
        })
      end
    end
  end,
  [MSG_SCENE_TEARDOWN] = function (data)
    if module.scenes[data.scene] ~= nil and module.current_scene ~= nil then
      if module.scene_instance ~= nil then
        module.scene_instance:destroy()
        module.scene_instance = nil
      end

      for _, system in ipairs(module.current_scene.systems) do
        lua_bridge.post_message("systems", MSG_SYSTEM_STOP, {
          ["system"] = system,
        })
      end

      module.current_scene = nil
      module.systems = {}
    end
  end,

  [MSG_SYSTEM_SPUN_UP] = function (data)
    if module.current_scene ~= nil then
      module.systems[#module.systems + 1] = data.system

      local missing = false
      for _, scene_system in ipairs(module.current_scene.systems) do
        local found = false
        for _, spun_up_systems in ipairs(module.systems) do
          if scene_system == spun_up_systems then
            found = true
          end
        end

        if not found then
          missing = true
          break
        end
      end

      if not missing then
        module.scene_instance = module.current_scene()
      end
    end
  end,
}

function module.register(name, scene)
  module.scenes[name] = scene
end

function module.handle_message(msg, data)
  if module.events[msg] ~= nil then
    module.events[msg](data)
  end
end

return module

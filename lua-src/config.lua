resolution(1280, 720)
--fullscreen()
--gl_debug()

ascii_resolution(160, 90);
--frame_lock(60);

bind("game:kill", KEY_ESCAPE)
bind("scene:prev", KEY_LEFT)
bind("scene:next", KEY_RIGHT)

bind("player:move_up_left", KEY_KP_7)
bind("player:move_up", KEY_KP_8)
bind("player:move_up_right", KEY_KP_9)
bind("player:move_left", KEY_KP_4)
bind("player:move_right", KEY_KP_6)
bind("player:move_down_left", KEY_KP_1)
bind("player:move_down", KEY_KP_2)
bind("player:move_down_right", KEY_KP_3)

bind("scene:pause_updates", KEY_PAUSE);

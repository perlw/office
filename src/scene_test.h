#pragma once

typedef void SceneTest;

SceneTest *scene_test_create(Screen *screen, const Config *config);
void scene_test_destroy(SceneTest *scene);
void scene_test_update(SceneTest *scene, double delta);
void scene_test_draw(SceneTest *scene);

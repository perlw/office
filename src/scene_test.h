#pragma once

typedef void SceneTest;

// +SceneTest
SceneTest *scene_test_create(const Config *config);
void scene_test_destroy(SceneTest *scene);
void scene_test_update(SceneTest *scene, double delta);
void scene_test_draw(SceneTest *scene);
// -SceneTest

// +SceneTest2
SceneTest *scene_test2_create(const Config *config);
void scene_test2_destroy(SceneTest *scene);
void scene_test2_update(SceneTest *scene, double delta);
void scene_test2_draw(SceneTest *scene);
// -SceneTest2

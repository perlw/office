#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "glad/glad.h"

#include "occulus/occulus.h"

#include "picasso.h"

struct PicassoShader {
	uint32_t id;
	GLenum type;
};

struct PicassoProgram {
	uint32_t id;
};

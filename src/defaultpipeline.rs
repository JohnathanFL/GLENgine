
pub mod defaultvs {
    #[derive(VulkanoShader)]
    #[ty = "vertex"]
    #[src = "\
#version 450
layout(location = 0) in vec3 pos;
layout(location = 1) in uint data;

layout(location = 0) out uint voxel_out;

layout(set = 0, binding = 0) uniform PerFrameInfo {
    float timeStep;
    mat4 persp;
    mat4 camPos;
} global;

layout(set = 0, binding = 1) uniform PerMeshInfo {
    mat4 transform;
} local;

void main() {
    voxel_out = data;
    gl_Position = global.persp * global.camPos * local.transform * vec4(pos, 1.0);
}"]
    struct Dummy;
}

pub mod defaultfs {
    #[derive(VulkanoShader)]
    #[ty = "fragment"]
    #[src = "
#version 450
layout(location = 0) in flat uint data;
layout(location = 0) out vec4 f_color;

void main() {
    f_color = vec4(1.0);
}
"]
    struct Dummy;
}
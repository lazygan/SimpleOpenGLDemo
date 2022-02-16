#version 330 core
in VS_OUT {
  in vec4 frag_pos;
	in vec2 frag_uv;
	in vec3 frag_norm;
} fs_in;

out vec4 color;

uniform sampler2D tex0;
uniform vec3 light_ambient;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 camera_pos;

void main()
{
    vec3 obj_color = texture(tex0, vec2(fs_in.frag_uv.x, 1-fs_in.frag_uv.y)).rgb;
    vec3	ambient = light_ambient;
    vec3	light_dir = normalize(light_pos - fs_in.frag_pos.xyz);
    vec3	normal = normalize(fs_in.frag_norm);
    float	diff_factor = max(dot(light_dir, normal), 0.0);
    vec3	diffuse = diff_factor * light_color;
    vec3	view_dir = normalize(camera_pos- fs_in.frag_pos.xyz);
    vec3  half_dir= normalize(light_dir + view_dir);
		float spec_factor = pow(max(dot(half_dir, normal), 0.0), 32.0); // 32.0为镜面高光系数
    vec3	specular = spec_factor * light_color;
    vec3	res_color = (ambient + diffuse + specular ) * obj_color;
    color = vec4(res_color, 1.0f);
} 
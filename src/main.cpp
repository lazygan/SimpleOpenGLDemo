#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <array>
#include "OBJ_Loader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Shader.hpp"
#include <memory>

using namespace std;

int width = 1024, height = 760;
const string model_path_prefix = resource_path + "model/";

struct Texture{
    int w = 1, h = 1, c = 3; // width , height, channels
    unsigned int tex_id;
    Texture(string filename =""){
        if(filename == ""){
            unsigned char *data = new unsigned char(0);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            delete data;
            return;
        }
        filename = model_path_prefix + filename;
        glGenTextures(1, & tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        // 为当前绑定的纹理对象设置环绕、过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 加载并生成纹理
        unsigned char *data = stbi_load(filename.c_str(), &w, &h, &c, 0);
        cout << filename <<endl;
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
    }
};
struct Material{
    Shader shader;
    vector<Texture> texes;
};

struct Mesh{
    enum attributes {pos, uv, normal};
    vector<glm::vec4> vertices;
    vector<glm::vec2> uvs;
    vector<glm::vec3> normals;
    vector<array<int,3>> triangles;
    shared_ptr<Material> mat;
    unsigned int vao, vbo, ebo;
    glm::mat4 Mm = glm::mat4(1.0f);
    void load_obj(string obj_fn){
        objl::Loader loader;
        obj_fn = model_path_prefix + obj_fn;
        if(!loader.LoadFile(obj_fn)){
            cout << "load obj fail" <<endl;
            exit(-1);
        }
        // for simplicity, assume only one mesh in .obj file
        objl::Mesh curMesh = loader.LoadedMeshes[0];
        cout << curMesh.Vertices.size() <<endl;
        cout << curMesh.Indices.size() <<endl;
        for (int j = 0; j < curMesh.Vertices.size(); j++) {
            auto v = curMesh.Vertices[j];
            auto pos = v.Position;
            vertices.push_back({pos.X, pos.Y, pos.Z, 1});
            auto tex_coord = v.TextureCoordinate;
            uvs.push_back({tex_coord.X, tex_coord.Y});
            auto norm = v.Normal;
            normals.push_back({norm.X, norm.Y, norm.Z});
        }
        for (int j = 0; j < curMesh.Indices.size(); j += 3) {
            triangles.push_back({(int) curMesh.Indices[j],(int)  curMesh.Indices[j + 1],(int)  curMesh.Indices[j + 2]});
        }
    }
    void mesh_to_gl(){
        glGenVertexArrays(1, &(vao));
        glGenBuffers(1, &(vbo));
        glGenBuffers(1, &(ebo));
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        int ver_size = sizeof(glm::vec4) * vertices.size();
        int uv_size = sizeof(glm::vec2) * uvs.size();
        int norm_size = sizeof(glm::vec3) * normals.size();
        glBufferData(GL_ARRAY_BUFFER, ver_size + uv_size + norm_size, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, ver_size, &vertices[0]);
        glBufferSubData(GL_ARRAY_BUFFER, ver_size, uv_size, &uvs[0]);
        glBufferSubData(GL_ARRAY_BUFFER, ver_size + uv_size, norm_size, &normals[0]);
        glVertexAttribPointer(Mesh::pos, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
        glVertexAttribPointer(Mesh::uv, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (const GLvoid*)(ver_size));
        glVertexAttribPointer(Mesh::normal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (const GLvoid*)(ver_size + uv_size));
        glEnableVertexAttribArray(Mesh::pos);
        glEnableVertexAttribArray(Mesh::uv);
        glEnableVertexAttribArray(Mesh::normal);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * triangles.size() * 3, &triangles[0], GL_STATIC_DRAW);
        glBindVertexArray(0);
    }
};

namespace camera {
    float fov = 60.0f, aspect = (float)width/ (float)height;
    glm::vec3 cameraPos = glm::vec3(0.0f,0.0,-8.f);
    glm::vec3 cameraFront= glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 cameraUp = glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 cameraRight(){ return glm::cross(cameraFront,cameraUp); }
    glm::mat4 Mv; // model view matrix
    glm::mat4 Mp; // perspective matrix
    void update_matrix(){
        glViewport(0, 0,width, height);
        aspect = (float)width/ (float)height;
        Mv= glm::lookAt(cameraPos, cameraPos+cameraFront, cameraUp);
        Mp= glm::perspective(glm::radians(fov/2), aspect, 0.01f, 100.0f);
    }
    void update_shader_camera(Shader& shader){
        update_matrix();
        shader.setMat4f("Mv",Mv);
        shader.setMat4f("Mp",Mp);
        shader.setVec3f("camera_pos", cameraPos);
    }
};

namespace light{
    glm::vec3 light_pos(-3.f,3.f,-3.f);
    glm::vec3 light_ambient(0.1f, 0.1f, 0.1f);
    glm::vec3 light_color(0.8f, 0.8f, 0.8f);
    void update_shader_light(Shader& shader){
        shader.setVec3f("light_pos",light_pos);
        shader.setVec3f("light_ambient",light_ambient);
        shader.setVec3f("light_color",light_color);
    }
};

namespace glut_event_control{
    int pre_x, pre_y, button;
	void mouse_click(int button_, int state, int x, int y){
        button =  button_;
        if(button == GLUT_RIGHT_BUTTON){
            if(state == GLUT_DOWN){
                pre_x = x, pre_y = y, button = GLUT_RIGHT_BUTTON;
            }
        }
    }
    glm::vec3 up(0,1,0);
    glm::vec3 up_normal(0,1,0);
    glm::vec3 up_reverse(0,-1,0);    
    const float rot_speed = 0.8f;
    const float PI = 3.1415926535;
    float yaw = 0.0, tilt = 0.0;
    void mouse_press_move(int x, int y){
        if(button != GLUT_RIGHT_BUTTON) return; 
        int dx = x - pre_x, dy = y - pre_y;
        tilt -= (float)dy / (float)height * rot_speed;
        yaw -= (float)dx / (float)width * rot_speed;
        // Ensures that tilt stays within a certain range
        while( tilt < (PI * -1) ) tilt += (2 * PI);
        while( tilt > PI ) tilt -= (2 * PI);
        if( (tilt > (0.5*PI)) || (tilt < (-0.5*PI))) up = up_reverse;
        else up = up_normal;
        camera::cameraFront = glm::normalize( glm::vec3(sinf(yaw) * cos(tilt), sin(tilt), cosf(yaw) * cos(tilt)) );
        glm::vec3 right = glm::cross(camera::cameraFront, up);
        camera::cameraUp = glm::cross( right, camera::cameraFront);
        pre_x = x, pre_y = y;
        glutPostRedisplay();
    }
    void key_press(unsigned char key, int x, int y){ 
        float s = 0.1f;
        if(key == 'w') camera::cameraPos += (s * camera::cameraFront);
        if(key == 's') camera::cameraPos -= (s * camera::cameraFront);
        if(key == 'd') camera::cameraPos += (s * camera::cameraRight());
        if(key == 'a') camera::cameraPos -= (s * camera::cameraRight());
        glutPostRedisplay();
    } 
    void reshape(int w, int h){ width = w, height = h; }
    void idle(){ glutSwapBuffers(); }
}

Mesh build_spot(){
        shared_ptr<Material> spot_mat = make_shared<Material>();
        spot_mat->shader = Shader("blinn_phong");
        spot_mat->texes.emplace_back("spot/spot_texture.png");
        Mesh spot_mesh;
        spot_mesh.load_obj("spot/spot_triangulated_good.obj");
        spot_mesh.mat = spot_mat;
        return spot_mesh;
}

namespace render{
    vector<Mesh> meshes;
    void render_config(){
        glEnable(GL_DEPTH_TEST);
        meshes.push_back(build_spot());
        for(Mesh& mesh : meshes){
            mesh.mesh_to_gl();
        }
    }
    void render(){
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for(Mesh& mesh : meshes){
            Shader& shader = mesh.mat->shader;
            shader.use();
            light::update_shader_light(shader);
            camera::update_shader_camera(shader);
            shader.setMat4f("Mm",mesh.Mm);
            const vector<Texture>& texes = mesh.mat->texes;
            for(int i = 0; i < texes.size(); i++){
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, texes[i].tex_id);
                shader.setInt("tex" + to_string(i), i);
            }
            glBindVertexArray(mesh.vao);
            glDrawElements(GL_TRIANGLES, (GLsizei)mesh.triangles.size() * 3, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        glFlush();
        glutSwapBuffers();
    }
};

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Demo");
    GLenum err = glewInit();
	if (err != GLEW_OK) {
		cout << "glewInit Error:" << glewGetErrorString << endl;
        exit(-1);
	}
	glutReshapeFunc(glut_event_control::reshape);
	//glutIdleFunc(glut_event_control::idle);
	glutMouseFunc(glut_event_control::mouse_click);
	glutMotionFunc(glut_event_control::mouse_press_move);
	glutKeyboardFunc(glut_event_control::key_press);
    render::render_config();
	glutDisplayFunc(render::render);
    glutMainLoop();
	return 0;
}


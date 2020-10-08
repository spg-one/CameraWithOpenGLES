
#include <jni.h>
#include <android/log.h>

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  LOG_TAG    "native-lib"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
                                                    = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

auto gVertexShader = R"(#version 300 es
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec4 aTexCoord;
        uniform mat4 u_Matrix;
        out vec2 TexCoord;
        void main() {
        gl_Position = vec4(aPos, 1.0);
        gl_PointSize = 10.0;
        TexCoord = (u_Matrix * aTexCoord).xy;
        }
)";
auto gFragmentShader = R"(#version 300 es
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform samplerExternalOES texture1;
        void main() {
        FragColor = texture(texture1, TexCoord);
        }
)";

auto gFaceRectVertexShader = R"(#version 300 es
        layout (location = 0) in vec2 aPos;
        void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        gl_PointSize = 10.0;
        }
)";
auto gFaceRectFragmentShader = R"(#version 300 es
        precision mediump float;
        out vec4 FragColor;
        void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
)";

const float gVertices[] = {
        // positions         // texture coords
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f, // top right
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f  // top left
};

const unsigned short gIndices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
};

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %x:\n%s\n",
                         shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
GLuint gFaceRectProgram;
GLuint gMatrixLocation;
GLuint gInputTexture;
GLuint VBO, VAO, EBO, facePointsVbo;

bool setupGraphics(int w, int h, int tex) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d, %d)", w, h, tex);
    gInputTexture = tex;
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }

    gFaceRectProgram = createProgram(gFaceRectVertexShader, gFaceRectFragmentShader);
    if (!gFaceRectProgram) {
        LOGE("Could not create gFaceRectProgram.");
        return false;
    }

    gMatrixLocation = glGetUniformLocation(gProgram, "u_Matrix");
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &facePointsVbo);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gVertices), gVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), gIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    glBindVertexArray(0);
    return true;
}

void renderFrame(float *matrix) {
    glClearColor(0, 0, 0, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, gInputTexture);

    glUniformMatrix4fv(gMatrixLocation, 1, GL_FALSE, matrix);
    glBindVertexArray(VAO);
    checkGlError("glBindVertexArray");
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    checkGlError("glDrawElements");
}

void renderFaceRects(float *matrix, int pointsNum) {

    glUseProgram(gFaceRectProgram);
    checkGlError("glUseProgram");

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pointsNum, matrix, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_LINES, 0, pointsNum);
    checkGlError("glDrawArrays");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_david_BeautyRendering_NativeJNILib_init(JNIEnv *env, jclass clazz, jint width,
                                                 jint height, jint tex) {
    setupGraphics(width, height, tex);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_david_BeautyRendering_NativeJNILib_step(JNIEnv *env, jclass clazz, jfloatArray matrixValues) {
    jfloat* valuesjf = env->GetFloatArrayElements(matrixValues, 0);
    float* valuesf = valuesjf;
    renderFrame(valuesf);
}extern "C"
JNIEXPORT void JNICALL
Java_com_david_BeautyRendering_NativeJNILib_drawFaceRects(JNIEnv *env, jclass clazz,
                                                          jfloatArray face_points, int pointsNum) {
    jfloat* valuesjf = env->GetFloatArrayElements(face_points, 0);
    float* valuesf = valuesjf;
    renderFaceRects(valuesf, pointsNum);
}
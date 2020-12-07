//
// Created by Administrator on 2020/10/11.
//

#ifndef BEAUTYRENDERING_RENDERTEXTURE_H
#define BEAUTYRENDERING_RENDERTEXTURE_H

#include "GlesCommon.h"

class CRenderTexture {
public:
    CRenderTexture()
    {
        mProgram = 0;
    }
    void Init(int btnNumber)
    {
        if(mProgram)
        {
            //already init.
            return;
        }
        if(btnNumber == 1){
            LOGE("btnNumber == 1");
            mProgram = createProgram(mVertexShader, mFragmentShader);
        }
        else if(btnNumber == 3){
            LOGE("btnNumber == 3");
            mProgram = createProgram(mVertexShader, mFragmentShaderWithColorMap);
        }
        else if(btnNumber == 4){
            LOGE("btnNumber == 4");
            LOGE("program will detect face!");
            mProgram = createProgram(mVertexShader, mFragmentShader);
        }
        else if(btnNumber == 0){
            LOGE("attribute: btnNumber is set to default!");
        }
        else{
            LOGE("btn number wrong!");
        }

        if (!mProgram) {
            LOGE("Could not create program.");
        }

        mMatrixLocation = glGetUniformLocation(mProgram, "u_Matrix");
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

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

        glBindVertexArray(0);

        //——————————————————————————————创建ColorMap Texture————————————————————————————————//
        locColorTable = glGetUniformLocation(mProgram, "colorTable");

        glGenTextures(1, &colorMapTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, colorMapTexture);
        GLubyte textureData[11][3]= { {0, 0, 0},
                                      {28, 28, 28},
                                      {54, 54, 54},
                                      {79, 79, 79},
                                      {105, 105, 105},
                                      {130, 130, 130},
                                      {156, 156, 156},
                                      {181, 181, 181},
                                      {207, 207, 207},
                                      {232, 232, 232},
                                      {255, 255, 255}};
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     11, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

    }
    void Rendering(GLuint inputTexture, float *texMatrix, int btnNumber)
    {
        LOGI("RenderTexture Rendering.");
        Init(btnNumber);

        glClearColor(0, 0, 0, 1.0f);
        checkGlError("glClearColor");
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        checkGlError("glClear");

        glUseProgram(mProgram);
        checkGlError("glUseProgram");

        glUniform1i(locColorTable, 1);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, inputTexture);

        glUniformMatrix4fv(mMatrixLocation, 1, GL_FALSE, texMatrix);
        glBindVertexArray(VAO);
        checkGlError("glBindVertexArray");
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        checkGlError("glDrawElements");
        glBindVertexArray(0);
    }
private:
    GLuint mProgram;
    GLuint mMatrixLocation;
    GLuint mInputTexture;
    GLuint VBO, VAO, EBO;
    GLint locColorTable;
    GLuint colorMapTexture;

    const char* mVertexShader = R"(#version 300 es
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec4 aTexCoord;
        uniform mat4 u_Matrix;
        out vec2 TexCoord;
        void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = (u_Matrix * aTexCoord).xy;
        }
        )";
    const char* mFragmentShader = R"(#version 300 es
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform samplerExternalOES texture1;
        void main() {
        FragColor = texture(texture1, TexCoord);
        }
        )";
    const char* mFragmentShaderWithBlack = R"(#version 300 es
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform samplerExternalOES texture1;
        const highp vec3 W = vec3(0.2125, 0.7154, 0.0721);
        void main() {
        highp vec4 textureColor = texture(texture1, TexCoord);
        float luminance = dot(textureColor.rgb, W);
        FragColor = vec4(vec3(luminance), textureColor.a);
        }
        )";
    const char* mFragmentShaderWithColorMap = R"(#version 300 es
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform samplerExternalOES texture1;
        uniform sampler2D colorTable;
        void main() {
        float red = texture(texture1, TexCoord).r;
        float green = texture(texture1, TexCoord).g;
        float blue = texture(texture1, TexCoord).b;
        vec2 ColorCoord;
        ColorCoord.x = (red + green + blue)/3.0;
        ColorCoord.y = 0.0;
        FragColor = texture(colorTable, vec2((red + green + blue)/3.0, 0.0));
        }
        )";
};


#endif //BEAUTYRENDERING_RENDERTEXTURE_H

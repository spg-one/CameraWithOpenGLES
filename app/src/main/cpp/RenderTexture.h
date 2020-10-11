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
    void Init()
    {
        if(mProgram)
        {
            //already init.
            return;
        }
        mProgram = createProgram(mVertexShader, mFragmentShader);
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
    }
    void Rendering(GLuint inputTexture, float *texMatrix)
    {
        Init();

        glClearColor(0, 0, 0, 1.0f);
        checkGlError("glClearColor");
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        checkGlError("glClear");

        glUseProgram(mProgram);
        checkGlError("glUseProgram");

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
};


#endif //BEAUTYRENDERING_RENDERTEXTURE_H

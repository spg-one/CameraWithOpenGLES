//
// Created by Administrator on 2020/10/11.
//

#ifndef BEAUTYRENDERING_RENDERFACE_H
#define BEAUTYRENDERING_RENDERFACE_H


#include "GlesCommon.h"

class CRenderFace {
public:
    CRenderFace()
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

        glGenBuffers(1, &VBO);
    }
    void Rendering(float *matrix, int pointsNum)
    {
        LOGI("RenderTexture Rendering.");
        Init();

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(mProgram);
        checkGlError("glUseProgram");

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pointsNum * 2, matrix, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        checkGlError("glVertexAttribPointer");
        glEnableVertexAttribArray(0);

        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, pointsNum);
        checkGlError("glDrawArrays");
        glLineWidth(1.0f);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
private:
    GLuint mProgram;
    GLuint VBO;

    const char* mVertexShader = R"(#version 300 es
        layout (location = 0) in vec2 aPos;
        void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        }
        )";
    const char* mFragmentShader = R"(#version 300 es
        precision mediump float;
        out vec4 FragColor;
        void main() {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
        )";
};


#endif //BEAUTYRENDERING_RENDERFACE_H

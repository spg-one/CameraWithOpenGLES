//
// Created by Administrator on 2020/10/11.
//

#ifndef BEAUTYRENDERING_RENDERTEXTUREWITHBILATERAL_H
#define BEAUTYRENDERING_RENDERTEXTUREWITHBILATERAL_H

#include "GlesCommon.h"
#define GAUSSIAN_REPLACE_NUMBER 15

class CRenderTextureBilateral {
public:
    CRenderTextureBilateral()
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

        mWidthOffsetLocation = glGetUniformLocation(mProgram, "texelWidthOffset");
        mHeightOffsetLocation = glGetUniformLocation(mProgram, "texelHeightOffset");
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
    void Rendering(GLuint inputTexture, float *texMatrix, int width, int height)
    {
        LOGI("RenderTextureBilateral Rendering.");
        Init();

        glClearColor(0, 0, 0, 1.0f);
        checkGlError("glClearColor");
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        checkGlError("glClear");

        glUseProgram(mProgram);
        checkGlError("glUseProgram");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, inputTexture);

        glUniform1f(mWidthOffsetLocation, 1.0f/width);
        glUniform1f(mHeightOffsetLocation, 1.0f/height);
        glUniformMatrix4fv(mMatrixLocation, 1, GL_FALSE, texMatrix);
        glBindVertexArray(VAO);
        checkGlError("glBindVertexArray");
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        checkGlError("glDrawElements");
        glBindVertexArray(0);
    }
private:
    GLuint mProgram;
    GLuint mMatrixLocation, mWidthOffsetLocation, mHeightOffsetLocation;
    GLuint mInputTexture;
    GLuint VBO, VAO, EBO;

    const char *mVertexShader = R"(#version 300 es
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec4 aTexCoord;

        const int GAUSSIAN_SAMPLES = 15;
        uniform float texelWidthOffset;
        uniform float texelHeightOffset;
        uniform mat4 u_Matrix;
        out vec2 blurCoordinates[GAUSSIAN_SAMPLES];

        void main() {
            gl_Position = vec4(aPos, 1.0);

            // Calculate the positions for the blur
            int multiplier = 0;
            vec2 blurStep;
            vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);

            for (int i = 0; i < GAUSSIAN_SAMPLES; i++) {
                multiplier = (i - ((GAUSSIAN_SAMPLES - 1) / 2));
                // Blur in x (horizontal)
                blurStep = float(multiplier) * singleStepOffset;
                blurCoordinates[i] = (u_Matrix*(aTexCoord + vec4(blurStep, 0.0, 0.0))).xy;
            }
        }
        )";

    const char* mFragmentShader = R"(#version 300 es
        #extension GL_OES_EGL_image_external_essl3 : require
        precision mediump float;
        const int GAUSSIAN_SAMPLES = 15;
        const float distanceNormalizationFactor = 8.0;
        in vec2 blurCoordinates[GAUSSIAN_SAMPLES];
        uniform samplerExternalOES inputImageTexture;
        out vec4 FragColor;

        void main() {
             vec4 centralColor;
             float gaussianWeightTotal;
             vec4 sum;
             vec4 sampleColor;
             float distanceFromCentralColor;
             float gaussianWeight;

             centralColor = texture(inputImageTexture, blurCoordinates[GAUSSIAN_SAMPLES / 2]);
             gaussianWeightTotal = 0.18;
             sum = centralColor * 0.18;

             sampleColor = texture(inputImageTexture, blurCoordinates[0]);
             distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
             gaussianWeight = 0.05 * (1.0 - distanceFromCentralColor);
             gaussianWeightTotal += gaussianWeight;
             sum += sampleColor * gaussianWeight;

             sampleColor = texture(inputImageTexture, blurCoordinates[1]);
             distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
             gaussianWeight = 0.09 * (1.0 - distanceFromCentralColor);
             gaussianWeightTotal += gaussianWeight;
             sum += sampleColor * gaussianWeight;

             sampleColor = texture(inputImageTexture, blurCoordinates[2]);
             distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
             gaussianWeight = 0.12 * (1.0 - distanceFromCentralColor);
             gaussianWeightTotal += gaussianWeight;
             sum += sampleColor * gaussianWeight;

             sampleColor = texture(inputImageTexture, blurCoordinates[3]);
             distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
             gaussianWeight = 0.15 * (1.0 - distanceFromCentralColor);
             gaussianWeightTotal += gaussianWeight;
             sum += sampleColor * gaussianWeight;

             sampleColor = texture(inputImageTexture, blurCoordinates[5]);
             distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
             gaussianWeight = 0.15 * (1.0 - distanceFromCentralColor);
             gaussianWeightTotal += gaussianWeight;
             sum += sampleColor * gaussianWeight;

             sampleColor = texture(inputImageTexture, blurCoordinates[6]);
             distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
             gaussianWeight = 0.12 * (1.0 - distanceFromCentralColor);
             gaussianWeightTotal += gaussianWeight;
             sum += sampleColor * gaussianWeight;

             sampleColor = texture(inputImageTexture, blurCoordinates[7]);
             distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
             gaussianWeight = 0.09 * (1.0 - distanceFromCentralColor);
             gaussianWeightTotal += gaussianWeight;
             sum += sampleColor * gaussianWeight;

             sampleColor = texture(inputImageTexture, blurCoordinates[8]);
             distanceFromCentralColor = min(distance(centralColor, sampleColor) * distanceNormalizationFactor, 1.0);
             gaussianWeight = 0.05 * (1.0 - distanceFromCentralColor);
             gaussianWeightTotal += gaussianWeight;
             sum += sampleColor * gaussianWeight;

             FragColor = sum / gaussianWeightTotal;
        }
        )";
};




#endif //BEAUTYRENDERING_RENDERTEXTUREWITHBILATERAL_H

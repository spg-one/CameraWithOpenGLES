package com.david.BeautyRendering.render;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;

import com.david.BeautyRendering.NativeJNILib;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGLSurfaceView extends GLSurfaceView {

    private Camera2SurfaceRenderer mRenderer;

    public MyGLSurfaceView(Context context) {
        super(context);
        init();
    }
    public MyGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        setEGLContextClientVersion(3);

        // Set the Renderer for drawing on the GLSurfaceView
        mRenderer = new Camera2SurfaceRenderer();
        setRenderer(mRenderer);

        // Render the view only when there is a change in the drawing data
        //setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public SurfaceTexture getSurfaceTexture() {
        return mRenderer.getSurfaceTexture();
    }
}

class Camera2SurfaceRenderer implements GLSurfaceView.Renderer {

    private static final String TAG = "Camera2SurfaceRenderer";
    private int mProgram;

    private int textureId;

    private float[] transformMatrix = new float[16];
    /**
     * 顶点坐标
     * (x,y,z)
     */
    private float[] POSITION_VERTEX = new float[]{
            0f, 0f, 0f,     //顶点坐标V0
            1f, 1f, 0f,     //顶点坐标V1
            -1f, 1f, 0f,    //顶点坐标V2
            -1f, -1f, 0f,   //顶点坐标V3
            1f, -1f, 0f     //顶点坐标V4
    };

    /**
     * 纹理坐标
     * (s,t)
     */
    private static final float[] TEX_VERTEX = {
            0.5f, 0.5f, //纹理坐标V0
            1f, 1f,     //纹理坐标V1
            0f, 1f,     //纹理坐标V2
            0f, 0.0f,   //纹理坐标V3
            1f, 0.0f    //纹理坐标V4
    };

    /**
     * 索引
     */
    private static final short[] VERTEX_INDEX = {
            0, 1, 2,  //V0,V1,V2 三个顶点组成一个三角形
            0, 2, 3,  //V0,V2,V3 三个顶点组成一个三角形
            0, 3, 4,  //V0,V3,V4 三个顶点组成一个三角形
            0, 4, 1   //V0,V4,V1 三个顶点组成一个三角形
    };

    /**
     * 矩阵索引
     */
    private int uTextureMatrixLocation;

    private int uTextureSamplerLocation;

    private SurfaceTexture mSurfaceTexture;

    public Camera2SurfaceRenderer() {
    }


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        Log.e("Renderer", "onSurfaceCreated");
        //加载纹理
        textureId = loadTexture();
        mSurfaceTexture = new SurfaceTexture(textureId);
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        //GLES30.glViewport(0, 0, width, height);
        NativeJNILib.init(width, height, textureId);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        //更新纹理图像
        mSurfaceTexture.updateTexImage();
        mSurfaceTexture.getTransformMatrix(transformMatrix);

        NativeJNILib.step(transformMatrix);
    }


    public SurfaceTexture getSurfaceTexture() {
        Log.e(TAG, "mSurfaceTexture = " + mSurfaceTexture);
        return mSurfaceTexture;
    }

    /**
     * 加载外部纹理
     *
     * @return
     */
    public int loadTexture() {
        int[] tex = new int[1];
        //创建一个纹理
        GLES30.glGenTextures(1, tex, 0);
        //绑定到外部纹理上
        GLES30.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, tex[0]);
        //设置纹理过滤参数
        GLES30.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_NEAREST);
        GLES30.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
        GLES30.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);
        //解除纹理绑定
        GLES30.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
        return tex[0];
    }
}

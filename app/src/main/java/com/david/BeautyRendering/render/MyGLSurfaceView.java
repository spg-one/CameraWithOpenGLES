package com.david.BeautyRendering.render;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PointF;
import android.graphics.SurfaceTexture;
import android.media.FaceDetector;
import android.opengl.GLES11Ext;
import android.opengl.GLES30;
import android.opengl.GLException;
import android.opengl.GLSurfaceView;
import android.os.Environment;
import android.util.AttributeSet;
import android.util.Log;

import com.david.BeautyRendering.NativeJNILib;
import com.david.BeautyRendering.R;

import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.IntBuffer;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.opengles.GL10;


public class MyGLSurfaceView extends GLSurfaceView  implements GLSurfaceView.Renderer{

    private static final String TAG = "MyGLSurfaceView";
    private int textureId;

    private float[] transformMatrix = new float[16];

    private SurfaceTexture mSurfaceTexture;
    private Bitmap snapshotBitmap;
    static int frameIndexTmp = 0;

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
        setRenderer(this);

        // Render the view only when there is a change in the drawing data
        //setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
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

        if(++frameIndexTmp == 20)
        {
            frameIndexTmp = 0;

            captureBitmap(gl, new BitmapReadyCallbacks() {
                @Override
                public void onBitmapReady(Bitmap bitmap) {

                    int maxFace = 3;
                    FaceDetector detector = new FaceDetector(bitmap.getWidth(), bitmap.getHeight(), maxFace);
                    FaceDetector.Face[] faces = new FaceDetector.Face[maxFace];
                    int facesFound = detector.findFaces(bitmap, faces);
                    if(facesFound > 0)
                    {
                        for(int count=0;count<facesFound;count++)
                        {
                            FaceDetector.Face face = faces[count];
                            PointF midPoint=new PointF();
                            face.getMidPoint(midPoint);
                            float eyeDistance=face.eyesDistance();

                            float left = midPoint.x - (1.4f * eyeDistance);
                            float top = midPoint.y - (1.8f * eyeDistance);
                            float width = (2.8f * eyeDistance);
                            float height = (3.6f * eyeDistance);
                            Log.e(TAG, "face index:" + count + " pos(" + left + "," + top + ")  width = " + width + " height = " + height);
                        }
                    }
                    else
                    {
                        String filePath = Environment.getExternalStorageDirectory().getPath()+ '/' + "1.png";
                        Log.e(TAG, "face not found, imageSize(" + bitmap.getWidth() + "," + bitmap.getHeight() + ") save file = " + filePath);

                        try (FileOutputStream out = new FileOutputStream(filePath)) {
                            bitmap.compress(Bitmap.CompressFormat.PNG, 100, out); // bmp is your Bitmap instance
                            // PNG is a lossless format, the compression factor (100) is ignored
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                }
            });
        }

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


    private interface BitmapReadyCallbacks {
        void onBitmapReady(Bitmap bitmap);
    }

    private Bitmap convert(Bitmap bitmap, Bitmap.Config config) {
        Bitmap convertedBitmap = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(), config);
        Canvas canvas = new Canvas(convertedBitmap);
        Paint paint = new Paint();
        paint.setColor(Color.BLACK);
        canvas.drawBitmap(bitmap, 0, 0, paint);
        return convertedBitmap;
    }

    // supporting methods
    private void captureBitmap(final GL10 gl, final BitmapReadyCallbacks bitmapReadyCallbacks) {
        queueEvent(new Runnable() {
            @Override
            public void run() {
                snapshotBitmap = createBitmapFromGLSurface(0, 0, getWidth(), getHeight(), gl);

                //convert bitmap to RGB 565
                //Bitmap convertedBitmap = convert(snapshotBitmap, Bitmap.Config.RGB_565);

                bitmapReadyCallbacks.onBitmapReady(snapshotBitmap);

//                runOnUiThread(new Runnable() {
//                    @Override
//                    public void run() {
//                        bitmapReadyCallbacks.onBitmapReady(snapshotBitmap);
//                    }
//                });

            }
        });

    }

    private Bitmap createBitmapFromGLSurface(int x, int y, int w, int h, GL10 gl) {

        int bitmapBuffer[] = new int[w * h];
        int bitmapSource[] = new int[w * h];
        IntBuffer intBuffer = IntBuffer.wrap(bitmapBuffer);
        intBuffer.position(0);

        try {
            int glError = gl.glGetError();
            if(glError != GL10.GL_NO_ERROR)
            {
                Log.e(TAG, "createBitmapFromGLSurface: " + glError);
            }
            gl.glReadPixels(x, y, w, h, GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, intBuffer);
            glError = gl.glGetError();
            if(glError != GL10.GL_NO_ERROR)
            {
                Log.e(TAG, "createBitmapFromGLSurface: " + glError);
            }
            int offset1, offset2;
            for (int i = 0; i < h; i++) {
                offset1 = i * w;
                offset2 = (h - i - 1) * w;
                for (int j = 0; j < w; j++) {
                    int texturePixel = bitmapBuffer[offset1 + j];
                    int blue = (texturePixel >> 16) & 0xff;
                    int red = (texturePixel << 16) & 0x00ff0000;
                    int pixel = (texturePixel & 0xff00ff00) | red | blue;
                    bitmapSource[offset2 + j] = pixel;
                }
            }
        } catch (GLException e) {
            Log.e(TAG, "createBitmapFromGLSurface: " + e.getMessage(), e);
            return null;
        }

        return Bitmap.createBitmap(bitmapSource, w, h, Bitmap.Config.ARGB_8888);
    }
}
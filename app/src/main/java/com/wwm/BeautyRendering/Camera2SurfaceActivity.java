package com.wwm.BeautyRendering;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.util.Size;
import android.view.Surface;

import com.wwm.BeautyRendering.R;
import com.wwm.BeautyRendering.render.MyGLSurfaceView;

import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class Camera2SurfaceActivity extends Activity {
    private static final String TAG = "Camera2SurfaceActivity";
    private static CameraManager cameraManager;
    private int cameraId = 1; //前置摄像头
    List<Size> outputSizes;
    Size photoSize;
    CameraDevice cameraDevice;
    CameraCaptureSession captureSession;
    CaptureRequest.Builder previewRequestBuilder;
    CaptureRequest previewRequest;
    Surface surface;
    SurfaceTexture surfaceTexture;
    private MyGLSurfaceView myCamera2SurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_glsurface);
        requestPermission();

        myCamera2SurfaceView = findViewById(R.id.preview);
        myCamera2SurfaceView.btnCameraModeNumber = getIntent().getIntExtra("btnNumber",0);
        if (myCamera2SurfaceView == null) {
            Log.d(TAG, "mGLSurfaceView is null");
        }

    }

    private void requestPermission() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, 100);
        } else {
            initCamera();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 100 && grantResults != null && grantResults.length > 0) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                initCamera();
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        openCamera();
    }

    CameraCaptureSession.StateCallback sessionsStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(CameraCaptureSession session) {
            if (null == cameraDevice) {
                return;
            }

            captureSession = session;
            try {
                captureSession.setRepeatingRequest(previewRequest,
                        null,
                        null);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onConfigureFailed(CameraCaptureSession session) {
        }
    };


    CameraDevice.StateCallback cameraStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(CameraDevice camera) {
            surfaceTexture = myCamera2SurfaceView.getSurfaceTexture();
            if (surfaceTexture == null) {
                return;
            }

            surfaceTexture.setDefaultBufferSize(photoSize.getWidth(), photoSize.getHeight());
            Log.e(TAG, " " + photoSize.getWidth() + ":" + photoSize.getHeight());

            surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
                @Override
                public void onFrameAvailable(final SurfaceTexture surfaceTexture) {
                    myCamera2SurfaceView.requestRender();
                }
            });
            surface = new Surface(surfaceTexture);

            try {
                cameraDevice = camera;
                previewRequestBuilder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                previewRequestBuilder.addTarget(surface);
                previewRequest = previewRequestBuilder.build();

                cameraDevice.createCaptureSession(Arrays.asList(surface), sessionsStateCallback, null);
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onDisconnected(CameraDevice camera) {

        }

        @Override
        public void onError(CameraDevice camera, int error) {
            Log.e(TAG, "Open  onError");

        }
    };

    private void initCamera() {

        cameraManager = (CameraManager) MyApplication.getApplication().getSystemService(Context.CAMERA_SERVICE);
        //获取指定相机的输出尺寸列表
        outputSizes = getCameraOutputSizes(cameraId, SurfaceTexture.class);
//        for(int i=0; i<outputSizes.size(); ++i)
//        {
//            Log.e(TAG, "camera index = " + i + " size = (" + outputSizes.get(i).getWidth() + "," + outputSizes.get(i).getHeight() + ")");
//        }
        //photoSize = outputSizes.get(0);
        photoSize = new Size(1280, 960);  //use a default size.
    }

    @SuppressLint("MissingPermission")
    private void openCamera() {
        try {
            cameraManager.openCamera(String.valueOf(cameraId), cameraStateCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
            Log.e(TAG, "openCamera fail");
        }
    }


    //获取指定相机的输出尺寸列表，降序排序（第一个的清晰度最高，往后清晰度越低）
    public List<Size> getCameraOutputSizes(int cameraId, Class clz) {
        try {
            CameraCharacteristics characteristics = cameraManager.getCameraCharacteristics(String.valueOf(cameraId));
            StreamConfigurationMap configs = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            List<Size> sizes = Arrays.asList(configs.getOutputSizes(clz));
            Collections.sort(sizes, new Comparator<Size>() {
                @Override
                public int compare(Size o1, Size o2) {
                    return o1.getWidth() * o1.getHeight() - o2.getWidth() * o2.getHeight();
                }
            });
            Collections.reverse(sizes);
            return sizes;
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        return null;
    }
}

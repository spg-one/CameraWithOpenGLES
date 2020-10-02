package com.david.BeautyRendering;

public class NativeJNILib {
    static{
        System.loadLibrary("native-lib");
    }
    public static native void init(int width, int height, int tex);
    public static native void step(float []matrixValues);
}

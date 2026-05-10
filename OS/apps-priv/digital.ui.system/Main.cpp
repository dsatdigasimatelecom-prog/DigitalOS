#include <jni.h>
#include <dlfcn.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>

#define LOGI(...) __android_log_print(
    ANDROID_LOG_INFO,
    "DigitalOS",
    __VA_ARGS__
)

JavaVM* vm;

jclass viewClass;

jmethodID drawMethod;

jobject viewObject;

ANativeWindow* nativeWindow;

EGLDisplay display;
EGLSurface surface;
EGLContext context;

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(
    JavaVM* jvm,
    void* reserved
) {

    vm = jvm;

    return JNI_VERSION_1_6;
}

void initEGL() {

    display = eglGetDisplay(
        EGL_DEFAULT_DISPLAY
    );

    eglInitialize(
        display,
        0,
        0
    );

    EGLConfig config;

    EGLint numConfigs;

    EGLint attribs[] = {

        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES3_BIT,

        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,

        EGL_BLUE_SIZE,
        8,

        EGL_GREEN_SIZE,
        8,

        EGL_RED_SIZE,
        8,

        EGL_NONE
    };

    eglChooseConfig(
        display,
        attribs,
        &config,
        1,
        &numConfigs
    );

    EGLint ctxAttribs[] = {

        EGL_CONTEXT_CLIENT_VERSION,
        3,

        EGL_NONE
    };

    context = eglCreateContext(
        display,
        config,
        EGL_NO_CONTEXT,
        ctxAttribs
    );

    surface = eglCreateWindowSurface(
        display,
        config,
        nativeWindow,
        0
    );

    eglMakeCurrent(
        display,
        surface,
        surface,
        context
    );
}

void loadView(
    JNIEnv* env,
    const char* dexPath
) {

    jclass dexLoaderClass =
    env->FindClass(
        "dalvik/system/DexClassLoader"
    );

    jmethodID ctor =
    env->GetMethodID(
        dexLoaderClass,
        "<init>",
        "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V"
    );

    jobject loader =
    env->NewObject(
        dexLoaderClass,
        ctor,
        env->NewStringUTF(dexPath),
        env->NewStringUTF("/data/data/cache"),
        0,
        0
    );

    jmethodID loadClass =
    env->GetMethodID(
        dexLoaderClass,
        "loadClass",
        "(Ljava/lang/String;)Ljava/lang/Class;"
    );

    jobject clazz =
    env->CallObjectMethod(
        loader,
        loadClass,
        env->NewStringUTF(
            "digitalos.ui.View"
        )
    );

    viewClass =
    (jclass) env->NewGlobalRef(clazz);

    jmethodID constructor =
    env->GetMethodID(
        viewClass,
        "<init>",
        "()V"
    );

    viewObject =
    env->NewGlobalRef(
        env->NewObject(
            viewClass,
            constructor
        )
    );

    drawMethod =
    env->GetMethodID(
        viewClass,
        "draw",
        "()V"
    );
}

void renderLoop() {

    JNIEnv* env;

    vm->AttachCurrentThread(
        &env,
        0
    );

    while(true) {

        glViewport(
            0,
            0,
            1080,
            2400
        );

        glClearColor(
            0.1f,
            0.1f,
            0.1f,
            1.0f
        );

        glClear(
            GL_COLOR_BUFFER_BIT
        );

        env->CallVoidMethod(
            viewObject,
            drawMethod
        );

        eglSwapBuffers(
            display,
            surface
        );
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_digitalos_core_NativeBridge_boot(
    JNIEnv* env,
    jclass clazz,
    jobject surfaceObj,
    jstring dex
) {

    nativeWindow =
    ANativeWindow_fromSurface(
        env,
        surfaceObj
    );

    initEGL();

    const char* path =
    env->GetStringUTFChars(
        dex,
        0
    );

    loadView(
        env,
        path
    );

    renderLoop();
}

#include <jni.h>
#include "include/encoder.h" // Include your header for mpeg_encode_procedure

// JNI wrapper function
JNIEXPORT jint JNICALL Java_com_example_Encoder_mpegEncodeProcedure(
    JNIEnv *env, jobject obj, jstring imagesFolder, jstring bitstreamFolder, jstring videoPath) {

    // Convert Java strings to C strings
    const char *c_imagesFolder = (*env)->GetStringUTFChars(env, imagesFolder, 0);
    const char *c_bitstreamFolder = (*env)->GetStringUTFChars(env, bitstreamFolder, 0);
    const char *c_videoPath = (*env)->GetStringUTFChars(env, videoPath, 0);

    // Call the original C function
    int result = mpeg_encode_procedure(c_imagesFolder, c_bitstreamFolder, c_videoPath);

    // Release Java strings
    (*env)->ReleaseStringUTFChars(env, imagesFolder, c_imagesFolder);
    (*env)->ReleaseStringUTFChars(env, bitstreamFolder, c_bitstreamFolder);
    (*env)->ReleaseStringUTFChars(env, videoPath, c_videoPath);

    return result;
}

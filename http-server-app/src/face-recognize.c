/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if 0
#include <glib.h>
#include <stdlib.h>
#include <time.h>
#include <json-glib/json-glib.h>
#include <libsoup/soup.h>
#include <mv_common.h>
/* To use the functions and data types of the Media Vision Face API (in mobile and wearable applications), include the  <mv_face.h> header file in your application. */
#include <mv_face.h>
/* In addition, you must include the <image_util.h> header file to handle the image decoding tasks, or the  <camera.h> header file to provide preview images. */
/* Image decoding for face detection and recognition */
#include <image_util.h>

#define FILEPATH_SIZE 1024

/* For face recognition, use the following facedata_s structure: */
struct _facedata_s {
    mv_source_h g_source;
    mv_engine_config_h g_engine_config;
    mv_face_recognition_model_h g_face_recog_model;
};
typedef struct _facedata_s facedata_s;
static facedata_s facedata;

/* Add face examples to the face recognition model handle.
 * Make sure that the face examples are of the same person but captured at different angles.
 * The following example assumes that 10 face samples
 * (face_sample_1.jpg - face_sample_10.jpg in the <OwnDataPath> folder)
 * are used and that the face area in each example covers approximately 95~100% of the image.
 * The label of the face is set to ‘1’. */
int _make_face_recognize_model(void)
{
	int example_index = 0;
	int face_label = 1;

	char filePath[FILEPATH_SIZE] = {0, };
	unsigned char *dataBuffer = NULL;
	unsigned long long bufferSize = 0;
	unsigned long width = 0;
	unsigned long height = 0;
	image_util_decode_h imageDecoder = NULL;

	 error_code = image_util_decode_create(&imageDecoder);
	 retv_if(error_code != IMAGE_UTIL_ERROR_NONE, NULL);

	 error_code = image_util_decode_set_colorspace(imageDecoder, IMAGE_UTIL_COLORSPACE_RGB8888);
	 goto_if(error_code != IMAGE_UTIL_ERROR_NONE, ERROR);

	 for (example_index = 1; example_index <= 10; ++example_index) {
	    /* Decode image and fill the image data to g_source handle */
	    snprintf(filePath, FILEPATH_SIZE, "%s/face_sample_%d.jpg", mydir, example_index);
	    error_code = image_util_decode_set_input_path(imageDecoder, filePath);
	    if (error_code != IMAGE_UTIL_ERROR_NONE)
	        dlog_print(DLOG_ERROR, LOG_TAG, "error code = %d", error_code);

	    error_code = image_util_decode_set_output_buffer(imageDecoder, &dataBuffer);
	    if (error_code != IMAGE_UTIL_ERROR_NONE)
	        dlog_print(DLOG_ERROR, LOG_TAG, "error code = %d", error_code);

	    error_code = image_util_decode_run(imageDecoder, &width, &height, &bufferSize);
	    if (error_code != IMAGE_UTIL_ERROR_NONE)
	        dlog_print(DLOG_ERROR, LOG_TAG, "error code = %d", error_code);

	    roi.x = roi.y = 0;
	    roi.width = width;
	    roi.height = height;
	    error_code = mv_source_clear(facedata.g_source);
	    if (error_code != MEDIA_VISION_ERROR_NONE)
	        dlog_print(DLOG_ERROR, LOG_TAG, "error code = %d", error_code);

	    error_code = mv_source_fill_by_buffer(facedata.g_source, dataBuffer, (unsigned int)bufferSize,
	                                          (unsigned int)width, (unsigned int)height, MEDIA_VISION_COLORSPACE_RGBA);
	    if (error_code != MEDIA_VISION_ERROR_NONE)
	        dlog_print(DLOG_ERROR, LOG_TAG, "error code = %d", error_code);

	    free(dataBuffer);
	    dataBuffer = NULL;

	    error_code = mv_face_recognition_model_add(facedata.g_source,
	                                               facedata.g_face_recog_model, &roi, face_label);
	    if (error_code != MEDIA_VISION_ERROR_NONE)
	        dlog_print(DLOG_ERROR, LOG_TAG, "error code = %d", error_code);
	}

	error_code = image_util_decode_destroy(imageDecoder);
	if (error_code != IMAGE_UTIL_ERROR_NONE)
	    dlog_print(DLOG_ERROR, LOG_TAG, "error code = %d", error_code);

	return 0;

ERROR:
	return -1;
}

int face_recognize(void)
{
	int error_code = 0;

	/* Create the source and engine configuration handles: */
	error_code = mv_create_source(&facedata.g_source);
	retv_if(error_code != MEDIA_VISION_ERROR_NONE, -1);

	error_code = mv_create_engine_config(&facedata.g_engine_config);
	goto_if(error_code != MEDIA_VISION_ERROR_NONE, ERROR);

	/* Create a g_face_recog_model media vision face recognition model handle using the  mv_face_recognition_model_create() function.
	 * The handle must be created before any recognition is attempted. */
	error_code = mv_face_recognition_model_create(&facedata.g_face_recog_model);
	goto_if(error_code != MEDIA_VISION_ERROR_NONE, ERROR);

	return 0;

ERROR:
	return -1;
}
#endif

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
/* Preview images for face tracking */
#include <camera.h>

#include "http-server-log-private.h"
#include "http-server-route.h"
#include "hs-util-json.h"
#include "face-detect.h"
#include "image-cropper.h"

/* Face Detect Model from Tizen */
#define FACE_DETECT_MODEL_FILEPATH "/usr/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml"

/* For face detection, use the following facedata_s structure: */
struct _facedata_s {
    mv_source_h g_source;
    mv_engine_config_h g_engine_config;
};
typedef struct _facedata_s facedata_s;
static facedata_s facedata;

struct _face_detect_data_s {
	char *image_name;
	char *type;
	char *result;
	unsigned char *image_data;
	void *user_data;
	int size;
	int error;
	face_detect_result_cb callback;
};
typedef struct _face_detect_data_s face_detect_data_s;

/* The mv_face_detect() function invokes the _on_face_detected_cb() callback. */
static void _on_face_detected_cb(mv_source_h source, mv_engine_config_h engine_cfg,
	mv_rectangle_s *locations, int number_of_faces, void *user_data)
{
	face_detect_data_s *fd_data = user_data;
	int error_code = 0;

	ret_if(number_of_faces == 0);
	_D("Number of Faces : %d", number_of_faces);

	for (int i = 0; i < number_of_faces; ++i) {
		error_code = image_cropper_crop(fd_data->image_data,
				fd_data->size,
				locations[i].point.x,
				locations[i].point.y,
				locations[i].point.x + locations[i].width,
				locations[i].point.y + locations[i].height,
				"/home/owner/apps_rw/org.tizen.httpserver/data/face_sample_0.jpg");
		continue_if(error_code);
	}
}

static void _unset_engine_config(void)
{
	if (!facedata.g_engine_config) return;
	mv_destroy_engine_config(facedata.g_engine_config);
	facedata.g_engine_config = NULL;
}

static int _set_engine_config(void)
{
	int error_code = 0;

	/* Create the media vision engine using the mv_create_engine_config() function.
	 * The function creates the g_engine_config engine configuration handle and configures it with default attributes. */
	error_code = mv_create_engine_config(&facedata.g_engine_config);
	retv_if(error_code != MEDIA_VISION_ERROR_NONE, -1);

	/* Face detection details can be configured by setting attributes to the engine configuration handle.
	 * In this use case, the MV_FACE_DETECTION_MODEL_FILE_PATH attribute is configured. */
	error_code = mv_engine_config_set_string_attribute(facedata.g_engine_config,
		MV_FACE_DETECTION_MODEL_FILE_PATH,
		FACE_DETECT_MODEL_FILEPATH);
	goto_if(error_code != MEDIA_VISION_ERROR_NONE, ERROR);

	return 0;

ERROR:
	_unset_engine_config();
	return -1;
}

static void _free_face_detect_data(void *user_data)
{
	face_detect_data_s *fd_data = user_data;
	free(fd_data->image_name);
	free(fd_data->image_data);
	free(fd_data->type);
	free(fd_data->result);
	free(fd_data);
}

static gboolean face_detect_callback_call(void *user_data)
{
	face_detect_data_s *fd_data = user_data;

	if (fd_data->callback)
		fd_data->callback(fd_data->error, fd_data->image_name,
			fd_data->result, fd_data->user_data);

	return FALSE;
}

static gpointer _create_thread(void *data)
{
	face_detect_data_s *fd_data = data;

	unsigned char *dataBuffer = NULL;
	unsigned long long bufferSize = 0;
	unsigned long width = 0;
	unsigned long height = 0;

	image_util_decode_h imageDecoder = NULL;
	int error_code = 0;

	/* Decode the image file from which the face is to be detected, and fill the g_source handle with the decoded raw data. */
	error_code = image_util_decode_create(&imageDecoder);
	retv_if(error_code != IMAGE_UTIL_ERROR_NONE, NULL);

	error_code = image_util_decode_set_input_buffer(imageDecoder, fd_data->image_data, fd_data->size);
	goto_if(error_code != IMAGE_UTIL_ERROR_NONE, ERROR);

	/* Sets the output buffer to which the decoded buffer will be written to. */
	/* Deprecated since 5.5. */
	error_code = image_util_decode_set_output_buffer(imageDecoder, &dataBuffer);
	goto_if(error_code != IMAGE_UTIL_ERROR_NONE, ERROR);

	error_code = image_util_decode_set_colorspace(imageDecoder, IMAGE_UTIL_COLORSPACE_RGBA8888);
	goto_if(error_code != IMAGE_UTIL_ERROR_NONE, ERROR);

	/* Starts decoding of the image and fills the output buffer set using image_util_decode_set_output_buffer(). */
	/* Deprecated since 5.5. Use image_util_decode_run2() instead. */
	error_code = image_util_decode_run(imageDecoder, &width, &height, &bufferSize);
	goto_if(error_code != IMAGE_UTIL_ERROR_NONE, ERROR);

	image_util_decode_destroy(imageDecoder);
	imageDecoder = NULL;

	/* Create a source handle using the mv_create_source() function with the mv_source_h member of the detection data structure as the out parameter: */
	/* The source stores the face to be detected and all related data. You manage the source through the source handle. */
	error_code = mv_create_source(&facedata.g_source);
	goto_if(error_code != MEDIA_VISION_ERROR_NONE, ERROR);

	/* Fill the dataBuffer to g_source */
	error_code = mv_source_fill_by_buffer(facedata.g_source, dataBuffer, bufferSize,
			(unsigned int)width, (unsigned int)height, MEDIA_VISION_COLORSPACE_RGBA);
	goto_if(error_code != MEDIA_VISION_ERROR_NONE, ERROR);

	free(dataBuffer);
	dataBuffer = NULL;

	error_code = _set_engine_config();
	goto_if(error_code, ERROR);

	/* When the source and engine configuration handles are ready, use the mv_face_detect() function to detect faces: */
	error_code = mv_face_detect(facedata.g_source, facedata.g_engine_config, _on_face_detected_cb, fd_data);
	goto_if(error_code != MEDIA_VISION_ERROR_NONE, ERROR);

	_D("After mv_face_detect()");

	/* After the face detection is complete, destroy the source and engine configuration handles using the  mv_destroy_source() and mv_destroy_engine_config() functions: */
	mv_destroy_source(facedata.g_source);
	facedata.g_source = NULL;
	_unset_engine_config();

	fd_data->error = 0;
	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
		face_detect_callback_call, fd_data, _free_face_detect_data);

	return NULL;

ERROR:
	if (imageDecoder)
		image_util_decode_destroy(imageDecoder);

	free(dataBuffer);

	if (facedata.g_source) {
		mv_destroy_source(facedata.g_source);
		facedata.g_source = NULL;
	}

	_unset_engine_config();

	fd_data->error = -1;
	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
		face_detect_callback_call, fd_data, _free_face_detect_data);

	return NULL;
}

int face_detect(const char *image_name, const unsigned char *image_data, unsigned int size,
		const char *image_type, face_detect_result_cb callback, void *user_data)
{
	face_detect_data_s *fd_data = NULL;
	GThread *th = NULL;

	retv_if(!image_data, -1);
	retv_if(size == 0, -1);
	retv_if(!callback, -1);

	fd_data = calloc(1, sizeof(face_detect_data_s));
	retv_if(!fd_data, -1);

	fd_data->image_name = strdup(image_name);
	fd_data->image_data = g_memdup(image_data, size);
	fd_data->size = size;
	fd_data->type = strdup(image_type);
	fd_data->callback = callback;
	fd_data->user_data = user_data;
	fd_data->result = NULL;

	th = g_thread_try_new(NULL, _create_thread, fd_data, NULL);
	retvm_if(!th, -1, "failed to create a thread");

	return 0;
}

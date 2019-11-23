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

#ifndef __FACE_DETECT_H__
#define __FACE_DETECT_H__

typedef void (*face_detect_result_cb)(int error, const char *image_name, const char *result, void *user_data);

int face_detect(const char *image_name, const unsigned char *image_data, unsigned int size,
		const char *image_type, face_detect_result_cb callback, void *user_data);

#endif /* __FACE_DETECT_H__ */


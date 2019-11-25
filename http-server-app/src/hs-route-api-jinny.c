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
#include <libsoup/soup.h>
#include <json-glib/json-glib.h>
#include <system_info.h>
#include "http-server-log-private.h"
#include "http-server-route.h"
#include "hs-util-json.h"

static void route_api_jinny_callback(SoupMessage *msg,
					const char *path, GHashTable *query,
					SoupClientContext *client, gpointer user_data)
{
	bool bool_val = false;
	char *str_val = NULL;
	char *response_msg = NULL;
	gsize resp_msg_size = 0;
	JsonBuilder *builder = NULL;

	if (msg->method != SOUP_METHOD_GET) {
		soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
		return;
	}

	builder = json_builder_new();
	json_builder_begin_object(builder);

	util_json_add_str(builder, "Name", "Jin");
	util_json_add_str(builder, "Age", "20");
	util_json_add_str(builder, "Height", "180 cm");
	util_json_add_str(builder, "Weight", "70 Kg");
	util_json_add_str(builder, "Address", "Seoul");
	util_json_add_str(builder, "Nationality", "Korea");

	json_builder_end_object(builder);

	response_msg = util_json_generate_str(builder, &resp_msg_size);
	g_clear_pointer(&builder, g_object_unref);

	soup_message_body_append(msg->response_body, SOUP_MEMORY_COPY,
					response_msg, resp_msg_size);
	g_clear_pointer(&response_msg, g_free);

	soup_message_headers_set_content_type(
						msg->response_headers, "application/json", NULL);

	soup_message_set_status(msg, SOUP_STATUS_OK);
}

int hs_route_api_jinny_init(void)
{
	return http_server_route_handler_add("/api/jinnyInfo",
				route_api_jinny_callback, NULL, NULL);
}

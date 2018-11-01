#pragma once
#include "CivetServer.h"
#include <cstring>
#include <stdio.h>
#include <iostream>
#include "ConfigPageHandler.h"
#include "Config.h"
#include "serial_radar.h"
#include "pthreaddefs.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
bool exitNow = false;

/* Copyright (c) 2013-2018 the Civetweb developers
 * Copyright (c) 2013 No Face Press, LLC
 * License http://opensource.org/licenses/mit-license.php MIT License
 */

 // Simple example program on how to use Embedded C++ interface.
extern Config *cfg;

	bool
		ConfigPageHandler::handleGet(CivetServer *server, struct mg_connection *conn)
	{
		// Config *cfg = new Config("config.conf");
		json j1;
		cfg->toJSON(j1);

		string pretty = string("");
		string print = j1.dump(4);
		FILE *fp = fopen("config.conf", "w");
		fprintf(fp, print.c_str());
		fclose(fp);


		cfg->webFormPrint(pretty);

		
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/html\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "<html><body>\r\n");
		mg_printf(conn, pretty.c_str());

		mg_printf(conn, "</body></html>\r\n");
		return true;
	}


	bool
		ExitHandler::handleGet(CivetServer *server, struct mg_connection *conn)
	{
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/plain\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "Bye!\n");
		exitNow = true;
		return true;
	}



	bool
		AHandler::handleAll(const char *method,
			CivetServer *server,
			struct mg_connection *conn)
	{
		std::string s = "";
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/html\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "<html><body>");
		mg_printf(conn, "<h2>This is the A handler for \"%s\" !</h2>", method);
		if (CivetServer::getParam(conn, "param", s)) {
			mg_printf(conn, "<p>param set to %s</p>", s.c_str());
		}
		else if (CivetServer::getParam(conn, "config", s)) {
			s.clear();
			cfg->webFormProcess(conn, s);

			mg_printf(conn, "<p>config set to %s</p>", s.c_str());

			if (CivetServer::getParam(conn, "TargetsMainRangeMargin",s))
				mg_printf(conn, "<p>data %s</p>", s.c_str());
		} 
		else if (CivetServer::getParam(conn, "server", s)) {
			s.clear();
			cfg->webFormProcess(conn, s);

			mg_printf(conn, "<p>server set to %s</p>", s.c_str());

		}
		else if (CivetServer::getParam(conn, "start", s)) {
			mg_printf(conn, "start Acquisition");
		} else 
		if (CivetServer::getParam(conn, "stop", s)) {
			mg_printf(conn, "stop Acwquisition");
		}

		else {
			mg_printf(conn, "<p>param not set</p>");
		}
		mg_printf(conn, "</body></html>\n");
		return true;
	}
	bool
		AHandler::handlePut(CivetServer *server, struct mg_connection *conn)
	{
		return handleAll("PUT", server, conn);
	}
	bool
		AHandler::handleGet(CivetServer *server, struct mg_connection *conn)
	{
		return handleAll("GET", server, conn);
	}
	bool
		AHandler::handlePost(CivetServer *server, struct mg_connection *conn)
	{
		return handleAll("POST", server, conn);
	}

	bool
		ABHandler::handleGet(CivetServer *server, struct mg_connection *conn)
	{
		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/html\r\nConnection: close\r\n\r\n");
		mg_printf(conn, "<html><body>");
		mg_printf(conn, "<h2>This is the AB handler!!!</h2>");
		mg_printf(conn, "</body></html>\n");
		return true;
	}



	bool
		FooHandler::handleGet(CivetServer *server, struct mg_connection *conn)
	{
		/* Handler may access the request info using mg_get_request_info */
		const struct mg_request_info *req_info = mg_get_request_info(conn);

		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/html\r\nConnection: close\r\n\r\n");

		mg_printf(conn, "<html><body>\n");
		mg_printf(conn, "<h2>This is the Foo GET handler!!!</h2>\n");
		mg_printf(conn,
			"<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>\n",
			req_info->request_method,
			req_info->request_uri,
			req_info->http_version);
		mg_printf(conn, "</body></html>\n");

		return true;
	}
	bool
		FooHandler::handlePost(CivetServer *server, struct mg_connection *conn)
	{
		/* Handler may access the request info using mg_get_request_info */
		const struct mg_request_info *req_info = mg_get_request_info(conn);
		long long rlen, wlen;
		long long nlen = 0;
		long long tlen = req_info->content_length;
		char buf[1024];

		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: "
			"text/html\r\nConnection: close\r\n\r\n");

		mg_printf(conn, "<html><body>\n");
		mg_printf(conn, "<h2>This is the Foo POST handler!!!</h2>\n");
		mg_printf(conn,
			"<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>\n",
			req_info->request_method,
			req_info->request_uri,
			req_info->http_version);
		mg_printf(conn, "<p>Content Length: %li</p>\n", (long)tlen);
		mg_printf(conn, "<pre>\n");

		while (nlen < tlen) {
			rlen = tlen - nlen;
			if (rlen > sizeof(buf)) {
				rlen = sizeof(buf);
			}
			rlen = mg_read(conn, buf, (size_t)rlen);
			if (rlen <= 0) {
				break;
			}
			wlen = mg_write(conn, buf, (size_t)rlen);
			if (wlen != rlen) {
				break;
			}
			nlen += wlen;
		}

		mg_printf(conn, "\n</pre>\n");
		mg_printf(conn, "</body></html>\n");

		return true;
	}

#define fopen_recursive fopen

	bool
		FooHandler::handlePut(CivetServer *server, struct mg_connection *conn)
	{
		/* Handler may access the request info using mg_get_request_info */
		const struct mg_request_info *req_info = mg_get_request_info(conn);
		long long rlen, wlen;
		long long nlen = 0;
		long long tlen = req_info->content_length;
		FILE * f;
		char buf[1024];
		int fail = 0;

#ifdef _WIN32
		_snprintf(buf, sizeof(buf), "D:\\somewhere\\%s\\%s", req_info->remote_user, req_info->local_uri);
		buf[sizeof(buf) - 1] = 0;
		if (strlen(buf) > 255) {
			/* Windows will not work with path > 260 (MAX_PATH), unless we use
			 * the unicode API. However, this is just an example code: A real
			 * code will probably never store anything to D:\\somewhere and
			 * must be adapted to the specific needs anyhow. */
			fail = 1;
			f = NULL;
		}
		else {
			f = fopen_recursive(buf, "wb");
		}
#else
		snprintf(buf, sizeof(buf), "~/somewhere/%s/%s", req_info->remote_user, req_info->local_uri);
		buf[sizeof(buf) - 1] = 0;
		if (strlen(buf) > 1020) {
			/* The string is too long and probably truncated. Make sure an
			 * UTF-8 string is never truncated between the UTF-8 code bytes.
			 * This example code must be adapted to the specific needs. */
			fail = 1;
			f = NULL;
		}
		else {
			f = fopen_recursive(buf, "w");
		}
#endif

		if (!f) {
			fail = 1;
		}
		else {
			while (nlen < tlen) {
				rlen = tlen - nlen;
				if (rlen > sizeof(buf)) {
					rlen = sizeof(buf);
				}
				rlen = mg_read(conn, buf, (size_t)rlen);
				if (rlen <= 0) {
					fail = 1;
					break;
				}
				wlen = fwrite(buf, 1, (size_t)rlen, f);
				if (wlen != rlen) {
					fail = 1;
					break;
				}
				nlen += wlen;
			}
			fclose(f);
		}

		if (fail) {
			mg_printf(conn,
				"HTTP/1.1 409 Conflict\r\n"
				"Content-Type: text/plain\r\n"
				"Connection: close\r\n\r\n");
		}
		else {
			mg_printf(conn,
				"HTTP/1.1 201 Created\r\n"
				"Content-Type: text/plain\r\n"
				"Connection: close\r\n\r\n");
		}

		return true;
	}



	bool
		WsStartHandler::handleGet(CivetServer *server, struct mg_connection *conn)
	{

		mg_printf(conn,
			"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
			"close\r\n\r\n");

		mg_printf(conn, "<!DOCTYPE html>\n");
		mg_printf(conn, "<html>\n<head>\n");
		mg_printf(conn, "<meta charset=\"UTF-8\">\n");
		mg_printf(conn, "<title>Embedded websocket example</title>\n");

#ifdef USE_WEBSOCKET
		/* mg_printf(conn, "<script type=\"text/javascript\"><![CDATA[\n"); ...
		 * xhtml style */
		mg_printf(conn, "<script>\n");
		mg_printf(
			conn,
			"var i=0\n"
			"function load() {\n"
			"  var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';\n"
			"  connection = new WebSocket(wsproto + '//' + window.location.host + "
			"'/websocket');\n"
			"  websock_text_field = "
			"document.getElementById('websock_text_field');\n"
			"  connection.onmessage = function (e) {\n"
			"    websock_text_field.innerHTML=e.data;\n"
			"    i=i+1;"
			"    connection.send(i);\n"
			"  }\n"
			"  connection.onerror = function (error) {\n"
			"    alert('WebSocket error');\n"
			"    connection.close();\n"
			"  }\n"
			"}\n");
		/* mg_printf(conn, "]]></script>\n"); ... xhtml style */
		mg_printf(conn, "</script>\n");
		mg_printf(conn, "</head>\n<body onload=\"load()\">\n");
		mg_printf(
			conn,
			"<div id='websock_text_field'>No websocket connection yet</div>\n");
#else
		mg_printf(conn, "</head>\n<body>\n");
		mg_printf(conn, "Example not compiled with USE_WEBSOCKET\n");
#endif
		mg_printf(conn, "</body>\n</html>\n");

		return 1;
	}

	bool
		ConfigHandler::handleGet(CivetServer *server, struct mg_connection *conn)
	{
		cout << "handleGet in the ConfigHandler" << endl;
		return true;
	}
	 bool
		 ConfigHandler::handlePost(CivetServer *server, struct mg_connection *conn)
	 {
		 cout << "handlePost in the ConfigHandler" << endl;
		 return true;
	 }

	 bool
		 OperationHandler::handleGet(CivetServer *server, struct mg_connection *conn)
	 {
		 cout << "handleGet in the OperationHandler" << endl;
		 /* Handler may access the request info using mg_get_request_info */
		 const struct mg_request_info *req_info = mg_get_request_info(conn);
		 mg_printf(conn,
			 "HTTP/1.1 200 OK\r\nContent-Type: "
			 "text/html\r\nConnection: close\r\n\r\n");

		 mg_printf(conn, "<html><body>\n");
		 //mg_printf(conn, "<h2>In the Operation Handler</h2>\n");
		 /*mg_printf(conn,
			 "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>\n",
			 req_info->request_method,
			 req_info->request_uri,
			 req_info->http_version);
		 */
		 if (0 == strcmp(req_info->request_uri, "/start")) {
			 start_acquisition = true;
			 mg_printf(conn, "Starting the acquisition");
		 } else if (0 == strcmp(req_info->request_uri, "/stop")) {
			 mg_printf(conn, "Stopping the acquisition");
			 stop_acquisition = true;
		 }
		 else {
			 mg_printf(conn, "Haven't got a clue why it's here");
		 }
		 //mg_printf(conn, "got it");
		 mg_printf(conn, "</body></html>\n");
		 return true;
	 }

// int main(int argc, char *argv[])
void *
config_server(void  *pbuf)
{
	Buf_t *buf = (Buf_t *)pbuf;
	const char *options[] = {
		"document_root", DOCUMENT_ROOT, "listening_ports", PORT, 0 };

	std::vector<std::string> cpp_options;
	for (int i = 0; i < (sizeof(options) / sizeof(options[0]) - 1); i++) {
		cpp_options.push_back(options[i]);
	}

	// CivetServer server(options); // <-- C style start
	CivetServer server(cpp_options); // <-- C++ style start


	// setup all the REST API Handlers

	ConfigPageHandler h_ex;
	server.addHandler(EXAMPLE_URI, h_ex);

	ExitHandler h_exit;
	server.addHandler(EXIT_URI, h_exit);

	AHandler h_a;
	server.addHandler("/a", h_a);

	ABHandler h_ab;
	server.addHandler("/a/b", h_ab);

	WsStartHandler h_ws;
	server.addHandler("/ws", h_ws);

	ConfigHandler h_config;
	server.addHandler("/config", h_config);

	OperationHandler h_start;
	server.addHandler("/start", h_start);

	OperationHandler h_stop;
	server.addHandler("/stop", h_stop);

	ConfigHandler h_server;
	server.addHandler("/server", h_server);


#ifdef NO_FILES
	/* This handler will handle "everything else", including
	 * requests to files. If this handler is installed,
	 * NO_FILES should be set. */
	FooHandler h_foo;
	server.addHandler("", h_foo);

	printf("See a page from the \"all\" handler at http://localhost:%s/\n", PORT);
#else
	FooHandler h_foo;
	server.addHandler("**.foo", h_foo);
	printf("Browse files at http://localhost:%s/\n", PORT);
#endif

	printf("Run example at http://localhost:%s%s\n", PORT, EXAMPLE_URI);
	printf("Exit at http://localhost:%s%s\n", PORT, EXIT_URI);

	while (!exitNow) {
#ifdef _WIN32
		Sleep(2000);
		pthread_mutex_lock(&buf->webgo);
#else
		sleep(1);
#endif
	}

	printf("Bye!\n");

	return 0;
}

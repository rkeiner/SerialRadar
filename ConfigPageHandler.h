#pragma once
#include "CivetServer.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/* Copyright (c) 2013-2018 the Civetweb developers
 * Copyright (c) 2013 No Face Press, LLC
 * License http://opensource.org/licenses/mit-license.php MIT License
 */

 // Simple example program on how to use Embedded C++ interface.



#define DOCUMENT_ROOT "."
#define PORT "8081"
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"

class ConfigPageHandler : public CivetHandler
{
public:
	bool
		handleGet(CivetServer *server, struct mg_connection *conn);
};
class ExitHandler : public CivetHandler
{
public:
	bool
		handleGet(CivetServer *server, struct mg_connection *conn);
};

class AHandler : public CivetHandler
{
private:
	bool
		handleAll(const char *method,
			CivetServer *server,
			struct mg_connection *conn);

public:
	bool
		handleGet(CivetServer *server, struct mg_connection *conn);
	bool
		handlePost(CivetServer *server, struct mg_connection *conn);
	bool
		handlePut(CivetServer *server, struct mg_connection *conn);
};

class ABHandler : public CivetHandler
{
public:
	bool
		handleGet(CivetServer *server, struct mg_connection *conn);
};
class FooHandler : public CivetHandler
{
public:
	bool
		handleGet(CivetServer *server, struct mg_connection *conn);
	bool
		handlePost(CivetServer *server, struct mg_connection *conn);

#define fopen_recursive fopen

	bool
		handlePut(CivetServer *server, struct mg_connection *conn);
};


class WsStartHandler : public CivetHandler
{
public:
	bool
		handleGet(CivetServer *server, struct mg_connection *conn);

};

class ConfigHandler : public CivetHandler
{
public:
	bool
		handleGet(CivetServer *server, struct mg_connection *conn);
	bool
		handlePost(CivetServer *server, struct mg_connection *conn);
};

class OperationHandler : public CivetHandler
{
public:
	bool
		handleGet(CivetServer *server, struct mg_connection *conn);

};

int config_server(int argc, char *argv[]);
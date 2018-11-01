// async_publish.cpp
//
// This is a Paho MQTT C++ client, sample application.
//
// It's an example of how to send messages as an MQTT publisher using the
// C++ asynchronous client interface.
//
// The sample demonstrates:
//  - Connecting to an MQTT server/broker
//  - Publishing messages
//  - Last will and testament
//  - Using asynchronous tokens
//  - Implementing callbacks and action listeners
//

/*******************************************************************************
 * Copyright (c) 2013-2017 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>	// For sleep
#include <atomic>
#include <chrono>
#include <cstring>
#include "mqtt/async_client.h"

using namespace std;

const std::string DFLT_SERVER_ADDRESS{ "tcp://162.250.208.202:1883" };
const std::string DFLT_CLIENT_ID{ "DASUser" };

const string TOPIC{ "test/fromradar" };


const auto TIMEOUT = std::chrono::seconds(10);

/////////////////////////////////////////////////////////////////////////////

/**
 * A callback class for use with the main MQTT client.
 */
class callback : public virtual mqtt::callback
{
public:
	void connection_lost(const string& cause) override {
		cout << "\nConnection lost" << endl;
		if (!cause.empty())
			cout << "\tcause: " << cause << endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr tok) override {
		cout << "\tDelivery complete for token: "
			<< (tok ? tok->get_message_id() : -1) << endl;
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A base action listener.
 */
class action_listener : public virtual mqtt::iaction_listener
{
protected:
	void on_failure(const mqtt::token& tok) override {
		cout << "\tListener failure for token: "
			<< tok.get_message_id() << endl;
	}

	void on_success(const mqtt::token& tok) override {
		//cout << "\tListener success for token: "
		//	<< tok.get_message_id() << endl;
	}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * A derived action listener for publish events.
 */
class delivery_action_listener : public action_listener
{
	atomic<bool> done_;

	void on_failure(const mqtt::token& tok) override {
		action_listener::on_failure(tok);
		done_ = true;
	}

	void on_success(const mqtt::token& tok) override {
		action_listener::on_success(tok);
		done_ = true;
	}

public:
	delivery_action_listener() : done_(false) {}
	bool is_done() const { return done_; }
};

/////////////////////////////////////////////////////////////////////////////
callback cb;

int async_publish(char * server, char * client, char *topic, char *payload, char * lastwill, int qos)
{
		string	address = (server == nullptr) ? DFLT_SERVER_ADDRESS : string(server);
		string clientID = (client == nullptr) ? DFLT_CLIENT_ID : string(client);

	try {
		static mqtt::async_client client(address, clientID);
		mqtt::connect_options conopts;
		mqtt::message willmsg(TOPIC, lastwill, 1, true);
		mqtt::will_options will(willmsg);
		conopts.set_will(will);

		client.set_callback(cb);
		//cout << "  +++OK" << endl;
		static mqtt::token_ptr conntok;
		if (!client.is_connected()){

			cout << "\nConnecting+++ to server " << address << endl;
			mqtt::token_ptr conntok = client.connect(conopts);
			conntok->wait();
			cout << "Waiting for the connection..." << endl;
		}
		else {
//			cout << "Already connected" << endl;
		}

		mqtt::delivery_token_ptr pubtok;

		mqtt::message_ptr pubmsg = mqtt::make_message(topic, payload);
		pubmsg->set_qos(qos);


		//cout << "Sending message  >>>" << endl;
		action_listener listener;
		pubmsg = mqtt::make_message(topic, payload);
		pubtok = client.publish(pubmsg, nullptr, listener);
		pubtok->wait();
		//cout << "  <<<< OK - Published" << endl;

		// Disconnect
		//cout << "\nDisconnecting..." << endl;
		//conntok = client.disconnect();
		//conntok->wait();
		//cout << "  ...OK" << endl;
	}
	catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
		return 1;
	}

	return 0;
}


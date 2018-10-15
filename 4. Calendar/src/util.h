#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <cstdint>
#include <iostream>
#include <booster/log.h>

#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h> 
#include <cppcms/url_dispatcher.h>  
#include <cppcms/url_mapper.h>  
#include <cppcms/applications_pool.h>
#include <cppcms/http_request.h>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;


using std::string;
using std::vector;


const string DB_NAME = "testdb";
const string DB_COLLECTION = "testcollection";


// expected list of fields for each event
const vector<string> FIELDS = {
	"description",
	"start",
	"end"
};


// check POST json to contain all fields from FIELDS
bool check_fields_need_all(const bsoncxx::v_noabi::document::view& view);

// check POST json fields to be only from FIELDS
bool check_fields_only_registered(const bsoncxx::v_noabi::document::view& view);

// check datetime string format given by user
bool check_time_string(const string& str);

// convert user POST json to mongodb document
bsoncxx::v_noabi::document::value post_to_bson(const std::pair<void*, long unsigned int>&);

// convert mongodb document to user view json
bsoncxx::document::value document_to_result(const string& uid, const bsoncxx::v_noabi::document::view& doc);

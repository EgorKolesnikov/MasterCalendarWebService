#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>
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
#include <bsoncxx/json.hpp>
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


class Calendar : public cppcms::application {
public:
    Calendar(cppcms::service &srv, std::shared_ptr<mongocxx::pool> mongo_pool) 
    	: cppcms::application(srv)
    	, _mongo_pool(mongo_pool) 
    {
    	dispatcher().map("POST", "/events/", &Calendar::event_create, this);
        dispatcher().map("GET", "/event/([0-9a-z]{24})/", &Calendar::event_get, this, 1);
        dispatcher().map("POST", "/event/([0-9a-z]{24})/", &Calendar::event_update, this, 1);

        // allowing "date_from" GET param 
        dispatcher().map("GET", "/events/", &Calendar::events_list_get, this);
    }

    virtual void event_create();
    virtual void event_get(std::string& uid);
    virtual void event_update(std::string& uid);
    virtual void events_list_get();

private:
	std::shared_ptr<mongocxx::pool> _mongo_pool;
};

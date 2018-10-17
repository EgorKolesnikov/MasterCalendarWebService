#include <cppcms/application.h>
#include <cppcms/applications_pool.h>
#include <cppcms/service.h>
#include <cppcms/http_response.h> 
#include <cppcms/url_dispatcher.h>  
#include <cppcms/url_mapper.h>  
#include <cppcms/applications_pool.h>
#include <booster/log.h> 

#include "util.h"
#include "urls.h"

#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;


int main(int argc, char ** argv){
    try {
        mongocxx::uri uri{"mongodb://localhost:27017/?minPoolSize=32&maxPoolSize=128"};
        std::shared_ptr<mongocxx::instance> mongo_instance = std::shared_ptr<mongocxx::instance>(new mongocxx::instance{});
        std::shared_ptr<mongocxx::pool> mongo_pool = std::shared_ptr<mongocxx::pool>(new mongocxx::pool{uri});

        cppcms::service srv(argc,argv);
        srv.applications_pool().mount(cppcms::applications_factory<Calendar>(mongo_pool));
        srv.run();
    }
    catch(std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
}

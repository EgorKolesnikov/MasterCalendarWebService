#include "urls.h"
#include "util.h"


void Calendar::event_create() {
    /*
    *   Creating new document in DB from given POST request data.
    *   To check:
    *    - POST should contain all registered keys and only them
    *    - check given dates formats
    */

    // get mongodb client
    auto client = this->_mongo_pool->acquire();
    auto collection = (*client)[DB_NAME][DB_COLLECTION];

    // convert post data to bson
    bsoncxx::v_noabi::document::value parsed_document = post_to_bson(request().raw_post_data());
    bsoncxx::v_noabi::document::view view = parsed_document.view();

    // check all fields should be present
    if(!check_fields_need_all(view)){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Not all fields specified\n";
        return;
    }

    // check there should not be other fields
    if(!check_fields_only_registered(view)){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Found not registered fields in POST json\n";
        return;
    }

    // check dates
    if(!check_time_string(view["start"].get_utf8().value.to_string()) || !check_time_string(view["end"].get_utf8().value.to_string())){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Invalid dates formats\n";
        return;
    }

    // copy, because we shoud use std::move() to insert
    bsoncxx::v_noabi::document::value copy{view};
    bsoncxx::v_noabi::document::view copy_view = copy.view();

    // insert and get uid
    bsoncxx::stdx::optional<mongocxx::result::insert_one> res = collection.insert_one(std::move(parsed_document));
    string uid = res->inserted_id().get_oid().value.to_string();

    // send result
    response().status(cppcms::http::response::created);
    response().out() << document_to_result(uid, copy_view);
}


void Calendar::event_get(std::string& uid) {
    /*
    *   Extracting one event by given uid.
    *   If not found - return 400 NOT_FOUND
    */

    // get mongodb client
    auto client = this->_mongo_pool->acquire();
    auto collection = (*client)[DB_NAME][DB_COLLECTION];

    // check given uid format
    // if bad format - return NOT FOUND code
    bsoncxx::oid oid;
    try{
        oid = bsoncxx::oid(uid);
    } catch(std::exception const &e) {
        std::cerr << e.what() << std::endl;
        response().status(cppcms::http::response::not_found);
        response().out() << "NOT FOUND " << uid;
        return;
    }

    // create filter by uid
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result = collection.find_one(document{} << "_id" << oid << finalize);

    // check if we found needed document
    if(!maybe_result) {
        response().status(cppcms::http::response::not_found);
        response().out() << "NOT FOUND " << uid;
        return;
    }

    // found it! send in strict format
    response().status(cppcms::http::response::ok);
    response().out() << document_to_result(uid, *maybe_result);
}


void Calendar::event_update(std::string& uid) {
    /*
    *   Updating specified document with new keys values.
    *   POST json dict may contain not all registered fields.
    *   To check:
    *    - POST keys should be only from registered fields list
    *    - if date is updated - check date format
    */

    // get mongodb client
    auto client = this->_mongo_pool->acquire();
    auto collection = (*client)[DB_NAME][DB_COLLECTION];

    // convert post data to bson
    bsoncxx::v_noabi::document::value parsed_document = post_to_bson(request().raw_post_data());
    bsoncxx::v_noabi::document::view view = parsed_document.view();

    // check there should not be other fields
    if(!check_fields_only_registered(view)){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Found not registered fields in POST json\n";
        return;
    }

    // check dates if they were given
    if(view["start"] && !check_time_string(view["start"].get_utf8().value.to_string())){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Invalid dates formats\n";
        return;
    }
    if(view["end"] && !check_time_string(view["end"].get_utf8().value.to_string())){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Invalid dates formats\n";
        return;
    }

    // check given uid format
    // if bad format - return NOT FOUND code
    bsoncxx::oid oid;
    try{
        oid = bsoncxx::oid(uid);
    } catch(std::exception const &e) {
        std::cerr << e.what() << std::endl;
        response().status(cppcms::http::response::not_found);
        response().out() << "NOT FOUND " << uid;
        return;
    }

    // DB query filter to search the document which we want to update
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp("_id", oid));

    // new document values to set
    bsoncxx::builder::basic::document update;
    update.append(
        bsoncxx::builder::basic::kvp("$set", [&view](bsoncxx::builder::basic::sub_document sd) {
            for (auto& element : view) {
                sd.append(bsoncxx::builder::basic::kvp(element.key(), element.get_value()));
            }
        })
    );

    // UPDATE DB query
    bsoncxx::stdx::optional<mongocxx::result::update> result = collection.update_one(filter.view(), update.view());

    // check query result
    if(!result || result->modified_count() == 0) {
        response().status(cppcms::http::response::not_found);
        response().out() << "NOT FOUND";
        return;
    }

    // updated!
    response().status(cppcms::http::response::ok);
    response().out() << "OK";
    return;
}


void Calendar::events_list_get() {
    /*
    *   Extract all registered events from DB.
    *   Convert result to JSON format: {"events": [{..}, {..}, {..}, ...]}
    */

    // get mongodb client
    auto client = this->_mongo_pool->acquire();
    auto collection = (*client)[DB_NAME][DB_COLLECTION];

    //
    //	Extract filter dates range from get params.
    //	Check extracted dates formats.
    //

    // default date
    std::string date_from = "1900-01-01T00:00:00Z";

    // extract given values from GET (may be empty)
    auto get_params = request().get();
    auto date_from_iter = get_params.find("date_from");

    // if GET param is not empty - rewrite default values with given value
    if(date_from_iter != get_params.end()){
    	date_from = date_from_iter->second;
    }

    // check date formats
    if(!check_time_string(date_from)){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Invalid filter date formats\n";
        return;
    }

    //
    //	Extracting all events with "date_from <= start"
    //

    mongocxx::options::find opts;
	// opts.sort(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("start", 1)));
	opts.limit(200);

    auto cursor = collection.find(
    	document{} 	<< "start"
    		   		<< open_document 
	    			<< "$gte" << date_from
		  			<< close_document
		  			<< finalize,
		opts
	);

    //
    //	Formatting result to json format
    //	Format: {"events": [{..}, {..}, ...]}
    //

    vector<std::string> events_json_strings;

    for (auto&& doc_view : cursor) {
        // collection may contain object with different keys
        // need to get only calendar event documents
        try{
            events_json_strings.push_back(document_to_result(doc_view["_id"].get_oid().value.to_string(), doc_view));
        }
        catch(std::exception const &e) {
            std::cerr << e.what() << std::endl;
        }
    }
    
    std::string joined = boost::algorithm::join(events_json_strings, ", ");
    std::string result = (boost::format("{\"events\": [%s]}") % joined).str();

    // send result
    response().status(cppcms::http::response::ok);
    response().out() << result;
}

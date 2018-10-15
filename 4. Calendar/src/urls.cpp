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
        std::cerr << "Not all fields specified\n" << std::endl;
        return;
    }

    // check there should not be other fields
    if(!check_fields_only_registered(view)){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Found not registered fields in POST json\n";
        std::cerr << "Found not registered fields in POST json\n" << std::endl;
        return;
    }

    // check dates
    if(!check_time_string(view["start"].get_utf8().value.to_string()) || !check_time_string(view["end"].get_utf8().value.to_string())){
        response().status(cppcms::http::response::bad_request);
        response().out() << "Invalid dates formats\n";
        std::cerr << "Invalid dates formats\n" << std::endl;
        return;
    }

    // copy, because we shoud use std::move() to insert
    bsoncxx::v_noabi::document::value copy{view};
    bsoncxx::v_noabi::document::view copy_view = copy.view();

    // insert and get uid
    bsoncxx::stdx::optional<mongocxx::result::insert_one> res = collection.insert_one(std::move(parsed_document));
    string uid = res->inserted_id().get_oid().value.to_string();

    // build the answer
    auto doc_value = document_to_result(uid, copy_view);

    // send result
    response().status(cppcms::http::response::ok);
    response().out() << bsoncxx::to_json(doc_value);
}


void Calendar::event_get(std::string& uid) {
    /*
    *   Extracting one event by given uid.
    *   If not found - return 400 NOT_FOUND
    */

    // get mongodb client
    auto client = this->_mongo_pool->acquire();
    auto collection = (*client)[DB_NAME][DB_COLLECTION];

    // create filter by uid
    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result = collection.find_one(document{} << "_id" << bsoncxx::oid(uid) << finalize);

    // check if we found needed document
    if(!maybe_result) {
    	std::cerr << uid << std::endl;
        response().status(cppcms::http::response::not_found);
        response().out() << "NOT FOUND " << uid;
        return;
    }

    // found it! send in strict format
    auto doc_value = document_to_result(uid, *maybe_result);
    response().status(cppcms::http::response::ok);
    response().out() << bsoncxx::to_json(doc_value);
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

    // DB query filter to search the document which we want to update
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp("_id", bsoncxx::oid(uid)));

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

    // searching for events
    auto cursor = collection.find({});

    // construct result json
    bsoncxx::builder::stream::document builder{};
    auto in_array = builder << "events" << bsoncxx::builder::stream::open_array;
    for (auto&& doc_view : cursor) {
        // collection may contain object with different keys
        // need to get only calendar event documents
        try{
            in_array = in_array << document_to_result(doc_view["_id"].get_oid().value.to_string(), doc_view).view();
        }
        catch(std::exception const &e) {
            std::cerr << e.what() << std::endl;
        }
    }
    bsoncxx::document::value doc = in_array << bsoncxx::builder::stream::close_array << bsoncxx::builder::stream::finalize;

    // send result
    response().status(cppcms::http::response::ok);
    response().out() << bsoncxx::to_json(doc);
}

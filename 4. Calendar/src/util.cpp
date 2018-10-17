#include "util.h"


bool check_time_string(const string& str){
    /*
    *   Check that given time string is in desired format: "%Y-%m-%dT%H:%M:%SZ"
    *   Using c++ <ctime> module (not cppcms or mongocxx)
    */
    struct tm tm;
    if (!strptime(str.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm)){
        return false;
    }
    return true;
}

int get_dates_difference_seconds(const string& str_time_1, const string& str_time_2){
	/*
	*	Calculate difference between two given dates.
	*	Dates given as strings in format %Y-%m-%dT%H:%M:%S
	*/
	struct tm tm_from;
    strptime(str_time_1.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm_from);

    struct tm tm_to;
    strptime(str_time_2.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm_to);

	std::time_t time1 = std::mktime(&tm_from);
	std::time_t time2 = std::mktime(&tm_to);

	return int(std::difftime(time2, time1));
}

std::string document_to_result(const string& uid, const bsoncxx::v_noabi::document::view& doc_view){
    /*
    *   Convert given mongodb document to the view, which will be show to user.
    *   Adding hypermedia and removing "_id" field
    */
    return (boost::format("{\"links\": {\"self\": \"/event/%s/\"}, \"event\": {\"description\": \"%s\", \"start\": \"%s\", \"end\": \"%s\"}}") %
    	uid
    	% doc_view["description"].get_utf8().value.to_string()
    	% doc_view["start"].get_utf8().value.to_string()
    	% doc_view["end"].get_utf8().value.to_string()
    ).str();
}

bsoncxx::v_noabi::document::value post_to_bson(const std::pair<void*, long unsigned int>& raw_post_data){
    /*
    *   Parse user POST body to json object.
    */
    return bsoncxx::from_json(std::string(reinterpret_cast<char const *>(raw_post_data.first), raw_post_data.second));
}


bool check_fields_need_all(const bsoncxx::v_noabi::document::view& view){
    /*
    *   Check that POST body contains ALL calenda event fields.
    */
    for(auto& target_key : FIELDS){
        auto parsed = view[target_key];
        if(!parsed){
            return false;
        }
    }
    return true;
}


bool check_fields_only_registered(const bsoncxx::v_noabi::document::view& view){
    /*
    *   Check that POST body contains fields only from registered list (util.FIELDS).
    *   May contain not all of them, but should contains only from that list.
    */
    for (auto& element : view) {
        // convert to string
        auto field_key{element.key()};
        std::string key_string = field_key.to_string();

        // search in registered fields
        if (std::find(FIELDS.begin(), FIELDS.end(), key_string) == FIELDS.end()){
            return false;
        }
    }
    return true;
}

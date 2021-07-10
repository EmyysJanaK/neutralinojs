#include <iostream>
#include <fstream>
#include <regex>
#include "lib/json.hpp"
#include "settings.h"
#include "../filesystem/filesystem.h"

#define STORAGE_DIR "/.storage"
#define STORAGE_EXT ".neustorage"
#define STORAGE_BUCKET_REGEX "^[a-zA-Z-_0-9]{2,50}$"

using namespace std;
using json = nlohmann::json;

namespace storage {
namespace controllers {
    json __validateStorageBucket(string bucketName) {
        if(regex_match(bucketName, regex(STORAGE_BUCKET_REGEX)))
            return nullptr;
        json output;
        output["error"] = "Invalid storage bucket identifer";
        return output;
    }
    
    json getData(json input) {
        json output;
        string bucket = input["bucket"];
        json errorPayload = __validateStorageBucket(bucket);
        if(!errorPayload.is_null()) 
            return errorPayload;
        string bucketPath = settings::joinAppPath(STORAGE_DIR);
        string filename = bucketPath + "/" + bucket + STORAGE_EXT;
        
        ifstream t;
        t.open(filename);
        if(!t.is_open()) {
            output["error"] = "Unable to open storage bucket: " + bucket;
            return output;
        }
        string buffer = "";
        string line;
        while(!t.eof()){
            getline(t, line);
            buffer += line + "\n";
        }
        t.close();
        output["data"] = buffer;
        output["success"] = true;
        return output;
    }

    json putData(json input) {
        json output;
        string bucket = input["bucket"];
        json errorPayload = __validateStorageBucket(bucket);
        if(!errorPayload.is_null()) 
            return errorPayload;
        string bucketPath = settings::joinAppPath(STORAGE_DIR);

        fs::createDirectory(bucketPath);
        
        string filename = bucketPath + "/" + bucket + STORAGE_EXT;
        if(input["data"].is_null()) {
            json readFileParams;
            readFileParams["fileName"] = filename;
            fs::removeFile(readFileParams);
        }
        else {
            string content = input["data"];
            ofstream t(filename);
            if(!t.is_open()) {
                output["error"] = "Unable to write storage bucket: " + bucket;
                return output;
            }
            t << content;
            t.close();
        }
        output["success"] = true;
        return output;
    }

} // namespace controllers
} // namespace storage

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include "json.hpp"
#include "httplib.h"

#include "Light.h"

using namespace lm;
using json = nlohmann::json;

const std::string URL = "http://localhost:8080";


void log_changes(const std::vector<std::string>& changes){
    for(auto& line : changes){
        std::cout << line << std::endl;
    }
}


int format_brightness(int brightness){
    //Convert brightness from 1-255 to 1-100
    return (int)brightness / 2.55;
}


void compare_settings(const Light& updated, const Light& current, std::vector<std::string>& changes) {
    if(current.name != updated.name) {
        json j;
        j["id"] = updated.id;
        j["name"] = updated.name;
        changes.push_back(j.dump(4));
    }
    if(current.brightness != updated.brightness) {
        json j;
        j["id"] = updated.id;
        j["brightness"] = format_brightness(updated.brightness);
        changes.push_back(j.dump(4));
    }
    if(current.on != updated.on) {
        json j;
        j["id"] = updated.id;
        j["on"] = updated.on;
        changes.push_back(j.dump(4));
    }
    if(current.room != updated.room) {
        json j;
        j["id"] = updated.id;
        j["room"] = updated.room;
        changes.push_back(j.dump(4));
    }
}


std::vector<std::string> compare_lights(const std::unordered_map<std::string,Light>& updated_lights, std::unordered_map<std::string,Light>& current_lights){
    std::vector<std::string> changes{};
    auto deleted_lights = current_lights;
   
    for(const auto& light : updated_lights){
        std::string id = light.first;
        if(current_lights.count(id) > 0){
            compare_settings(light.second, current_lights[id], changes);
        }
        else{
            //new light added
            json j = light.second;
            j["brightness"] = format_brightness(light.second.brightness);
            changes.push_back(j.dump(4));
        }
        deleted_lights.erase(id);
    }
    for(const auto& deleted_light : deleted_lights){
        changes.push_back(deleted_light.second.name + " (" + deleted_light.second.id + ") has been removed");
    }
    return changes;
}


void handle_http_error(const httplib::Error& err){
    std::cerr << "HTTP error: " << httplib::to_string(err) << std::endl;
    std::cout << "Press Enter To Retry" << std::endl;
    std::cin.get();
}


int main(int argc, char* argv[]){
    //Unit Testing Setup
    doctest::Context ctx;
    ctx.applyCommandLine(argc, argv);
    int res = ctx.run();
    if(ctx.shouldExit()){
        return res;
    }

    std::unordered_map<std::string,Light> current_lights = {};
    httplib::Client client(URL);
    while(true){
        httplib::Result res{};
        if((res = client.Get("/lights")) && res->status == httplib::StatusCode::OK_200) {
            try{
                auto updated_lights = json::parse(res->body);
                std::unordered_map<std::string,Light> updated_lights_details;
                
                bool success = true;
                for(const auto& light : updated_lights){
                    auto id = light["id"].template get<std::string>();
                    
                    if((res = client.Get("/lights/" + id)) && res->status == httplib::StatusCode::OK_200){
                        auto j = json::parse(res->body);
                        auto light = j.template get<Light>();
                        updated_lights_details.insert({light.id, light});
                    }
                    else {
                        success = false;
                        handle_http_error(res.error());
                    }
                } if(success){
                    auto changes = compare_lights(updated_lights_details, current_lights);
                    current_lights = updated_lights_details;
                    log_changes(changes);
                }
            } catch (json::exception e) {
                std::cout << e.what() << std::endl;
                std::cout << "Press Enter To Retry" << std::endl;
                std::cin.get();
            }
        } else {handle_http_error(res.error());}
    }
    return 0;
}



// UNIT TESTS //

bool check_equality(const std::vector<std::string>& changes1, const std::vector<std::string>& changes2){
    if(changes1.size() != changes2.size()){
        FAIL("\nsize 1:", changes1.size(), "\nsize 2: ", changes2.size());
        return false;
    }
    for(int i=0; i<changes1.size(); i++){
        if(changes1[i] != changes2[i]){
            FAIL("\nline 1: ", changes1[i], "\nline 2: ", changes2[i]);
            return false;
        }
    }
    return true;
}


TEST_CASE("add a light"){
    std::unordered_map<std::string,Light> current_lights{};
    std::unordered_map<std::string,Light> updated_lights{};
    auto light1 = Light{"Light1", "1", "kitchen", 255, true};
    updated_lights.insert({light1.id, light1});
    auto actual_changes = compare_lights(updated_lights, current_lights);
    std::vector<std::string> expected_changes{};
    light1.brightness = 100;
    json j = light1;
    expected_changes.push_back(j.dump(4));

    CHECK(check_equality(actual_changes, expected_changes));
}

TEST_CASE("delete a light"){
    std::unordered_map<std::string,Light> current_lights{};
    std::unordered_map<std::string,Light> updated_lights{};
    auto light1 = Light{"Light1", "1", "kitchen", 255, true};
    current_lights.insert({light1.id, light1});
    auto actual_changes = compare_lights(updated_lights, current_lights);
    std::vector<std::string> expected_changes{};
    expected_changes.push_back("Light1 (1) has been removed");

    CHECK(check_equality(actual_changes, expected_changes));
}

TEST_CASE("change a light (on/off)"){
    std::unordered_map<std::string,Light> current_lights{};
    std::unordered_map<std::string,Light> updated_lights{};
    auto light1 = Light{"Light1", "1", "kitchen", 255, true};
    current_lights.insert({light1.id, light1});
    light1.on = false;
    updated_lights.insert({light1.id, light1});
    auto actual_changes = compare_lights(updated_lights, current_lights);
    std::vector<std::string> expected_changes{};
    json j;
    j["id"] = "1";
    j["on"] = false;
    expected_changes.push_back(j.dump(4));

    CHECK(check_equality(actual_changes, expected_changes));
}

TEST_CASE("empty lights"){
    std::unordered_map<std::string,Light> current_lights{};
    std::unordered_map<std::string,Light> updated_lights{};

    auto actual_changes = compare_lights(updated_lights, current_lights);
    std::vector<std::string> expected_changes{};

    CHECK(check_equality(actual_changes, expected_changes));
}
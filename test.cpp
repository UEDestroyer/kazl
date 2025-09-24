#include "/mnt/500GB/kazl/staticlib.h"

#include "cpp-httplib/httplib.h"
#include "json-nlohmann/json.hpp"

class Main{
public:

    int sub(){
		httplib::Server svr;
		svr.set_mount_point("/", "./static");
		svr.Get("/", [](const httplib::Request &req, httplib::Response &res) {
		    std::cout << "Server started at http://localhost:8080\n";
		    res.set_content(read_file("static/main/index.html"), "text/html");
		    return;
		});
		svr.Post("/", [](const httplib::Request &req, httplib::Response &res) {
		    nlohmann::json json_data = nlohmann::json::parse(req.body);
		    nlohmann::json users = nlohmann::json::parse(read_file("static/data/users.json"));
		    std::cout << "Received data: " << json_data.dump(4) << std::endl;
		    if (users.contains(json_data["username"].get<std::string>()) && users[json_data["username"].get<std::string>()] == json_data["password"]) {
		        print(Sout{} << "User authenticated: " << json_data["username"]);
		        nlohmann::json messages = nlohmann::json::parse(read_file("static/data/messages.json"));
		        messages.push_back({
		            {"username", json_data["username"]},
		            {"toUsername", json_data["toUsername"]},
		            {"message", json_data["message"]}
		        });
		        write_file("static/data/messages.json", messages.dump(4));
		        res.set_content(read_file("static/main/index.html"), "text/html");
		    } else if (!users.contains(json_data["username"].get<std::string>())){
		        std::cout << "User authenticated2: " << json_data["username"] << std::endl;
		        users[json_data["username"].get<std::string>()] = json_data["password"];
		        write_file("static/data/users.json", users.dump(4));
		        nlohmann::json messages = nlohmann::json::parse(read_file("static/data/messages.json"));
		        messages.push_back({
		            {"username", json_data["username"]},
		            {"toUsername", json_data["toUsername"]},
		            {"message", json_data["message"]}
		        });
		        write_file("static/data/messages.json", messages.dump(4));
		    }
		    nlohmann::json response_json = {
		        {"username", json_data["username"]},
		        {"toUsername", json_data["toUsername"]},
		        {"message", json_data["message"]}
		    };
		    std::cout<<!users.contains(json_data["username"])<<"  "<<json_data["username"]<<"  "<<json_data["toUsername"]<<"  "<<json_data["message"]<<std::endl;
		    res.set_content(response_json.dump(), "application/json");
		    return;
		});
		svr.Post("/read", [](const httplib::Request &req, httplib::Response &res) {
		    nlohmann::json json_data = nlohmann::json::parse(req.body);
		    nlohmann::json messages = nlohmann::json::parse(read_file("static/data/messages.json"));
		    nlohmann::json answer = nlohmann::json::array();
		    for (const auto &message : messages) {
		        if (message["toUsername"] == json_data["username"]) {
		            answer.push_back({
		                {"message", message["message"]},
		                {"user", message["username"]}
		            });
		        }
		        if (message["username"] == json_data["username"]) {
		            answer.push_back({
		                {"message", message["message"]},
		                {"user", message["toUsername"]},
		                {"me", 1}
		            });
		        }
		    }
		    std::cout << "Messages for user " << json_data["username"] << ": " << answer.dump() << std::endl;
		    res.set_content(answer.dump(), "application/json");
		    return;
		});
		while (true){
		    svr.listen("0.0.0.0", 8080);
		}
        return 0;
    }
};

int main(){
    Main m;
    m.sub();
    return 0;
}
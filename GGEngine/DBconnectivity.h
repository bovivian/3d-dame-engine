#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

class DBconnectivity
{
private:
	
public:
	std::string playerPublicKey;

	DBconnectivity();
	~DBconnectivity();

	bool logIn();
	void writeInventoryDB(std::string item);
	std::vector<std::string> getInventoryDB();

};
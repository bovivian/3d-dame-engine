#pragma once
#include <iostream>
#include <vector>
#include <conio.h>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/array/view.hpp>
#include <bsoncxx/array/view.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/string/to_string.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/value.hpp>

#include "DBconnectivity.h"
#include "LogManager.h"
#include "SHA256.h"

extern LogManager * gLogManager;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::array;

mongocxx::instance inst{};
mongocxx::client conn{ mongocxx::uri{"API-KEY"} };
auto collection = conn["testdb"]["users"];

std::string getpass(bool show_asterisk = true)
{
	const char BACKSPACE = 8;
	const char RETURN = 13;

	std::string password;
	unsigned char ch = 0;

	while ((ch = _getch()) != RETURN)
	{
		if (ch == BACKSPACE)
		{
			if (password.length() != 0)
			{
				if (show_asterisk)
					std::cout << "\b \b";
				password.resize(password.length() - 1);
			}
		}
		else if (ch == 0 || ch == 224) // handle escape sequences
		{
			_getch(); // ignore non printable chars
			continue;
		}
		else
		{
			password += ch;
			if (show_asterisk)
				std::cout << '*';
		}
	}
	std::cout << std::endl;
	return password;
}

DBconnectivity::DBconnectivity()
{

}

DBconnectivity::~DBconnectivity()
{

}

bool DBconnectivity::logIn()
{
	std::string publicKey, password;
	bool login = false;
	do {
		auto cursor = collection.find({});
		// Get request for the user identify
		std::cout << "Please provide your public key: ";
		std::cin >> publicKey;

		// hide onput for password
		std::cout << "Please provide your password: ";
		password = getpass(true); // true to shows the asterisk, false doesn't shows anything

		// hashing the password
		SHA256 sha;
		sha.update(password);
		uint8_t * digest = sha.digest();

		// pause to tests. can be deleted
		std::system("pause");

		for (auto doc : cursor)
		{
			bsoncxx::document::element publicKeyFromDB = doc["wallet"];
			if (publicKey == publicKeyFromDB.get_utf8().value)
			{
				bsoncxx::document::element passwordFromDB = doc["password"];
				if (SHA256::toString(digest) == passwordFromDB.get_utf8().value)
				{
					system("CLS");
					gLogManager->AddMessage("INFO: Login success.");
					login = true;
					break;
				}
				else
				{
					system("CLS");
					gLogManager->AddMessage("WARNING: Wrong wallet or password.");
				}
			}
			else
			{
				system("CLS");
				gLogManager->AddMessage("WARNING: Wrong wallet or password.");
			}
		}
		delete[] digest;
	} while (login == false);

	if (login)
	{
		playerPublicKey = publicKey;
		return true;
	}
	else
	{
		return false;
	}
}

void DBconnectivity::writeInventoryDB(std::string item)
{	
	auto arr = array{};

	arr << item;

	document filter_builder, update_builder;
	filter_builder << "wallet" << playerPublicKey;
	update_builder << "$push" << open_document << "items" << open_document
		<< "$each" << arr.view() << close_document << close_document;

	collection.update_one(filter_builder.view(), update_builder.view());
}

std::vector<std::string> DBconnectivity::getInventoryDB()
{
	std::vector<std::string> items;
	auto cursor = collection.find({});

	for (auto doc : cursor)
	{
		bsoncxx::document::element publicKeyFromDB = doc["wallet"];
		if (playerPublicKey == publicKeyFromDB.get_utf8().value)
		{
			bsoncxx::document::element itemsFromDB = doc["items"];
			if (itemsFromDB && itemsFromDB.type() == bsoncxx::type::k_array)
			{
				bsoncxx::array::view subarr{ itemsFromDB.get_array().value };
				for (bsoncxx::array::element ele : subarr) {
					items.push_back(bsoncxx::string::to_string(ele.get_utf8().value));
				}
			}
		}
	}
	return std::move(items);
}

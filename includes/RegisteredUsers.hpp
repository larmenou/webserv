#pragma once

#include <map>
#include <string>
#include <iostream>

class RegisteredUsers
{
	private:
		std::map<std::string, std::string> _db;

	public:
		RegisteredUsers();
		~RegisteredUsers();

		void addDb(std::string &str);
		const std::map<std::string, std::string> &getDb() const;
		bool authenticate(std::string str);
};
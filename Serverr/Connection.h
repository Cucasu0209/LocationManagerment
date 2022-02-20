#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

sql::Connection *conSingleton;
#define DB_HOST "127.0.0.1:3306"   //ip and port mysql 
#define DB_USER "root"
#define DB_PASSWORD "root"
#define DB_NAME "location_management"  //database name
/*
* @function getDbConnection : get a connection to database
*
* @return :  an instance of sql::Connection *
*
*/
sql::Connection * getDbConnection() {
	if (conSingleton != NULL && conSingleton->isValid() && conSingleton->isClosed() == false) {
		return conSingleton;
	}
	try {
		std::cout << "Get new DB connection!" << std::endl;
		sql::Driver *driver;
		sql::Connection *con;
		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect(DB_HOST, DB_USER, DB_PASSWORD);
		/* Connect to the MySQL database */
		con->setSchema(DB_NAME);
		conSingleton = con;
		return con;
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
	}
}



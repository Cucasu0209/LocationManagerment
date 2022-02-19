#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

sql::Connection *conSingleton;

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
		con = driver->connect("127.0.0.1:3306", "root", "root");
		/* Connect to the MySQL database */
		con->setSchema("location_management");
		conSingleton = con;
		return con;
	}
	catch (sql::SQLException &e) {
		std::cout << "# ERR: " << e.what() << std::endl;
	}
}



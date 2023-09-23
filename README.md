# google-clone
<img width="741" alt="Screenshot 2023-09-22 at 3 15 58 PM" src="https://github.com/zhihuizhang0625/google-clone/assets/85580892/eac71bf6-9d79-465b-8b44-6799ba1723f7">

### Overview
It is a multi-threaded Web server that provides simple searching and file viewing utilities. The project allows the server to read from files, and parse those files to record any words that show up in those files and implement parts of the web server to allow connections and handle Http requests and successfully built a functioning web server.

### FileReader
In this file, we implemented a simple file reader. The name of the file will be read at the time of construction, and the function read_file will read the entire contents of the file into a singular string. Either POSIX, the C FILE interface, or a stream from C++, can be used to implement the reader.

### WordIndex
In these files, we implemented a data-structure that will allow us to record which documents contain a word and how many times each of those words are contained in each document. we used STL containers to implement this.

### CrawlFileTree
This function will take in a file name and a WordIndex. It is up to this function to read the specified file, and record each word that is found in it. This file is the core of our file processing that we will later use for search engine results for the server.

### ServerSocket
This file contains a helpful class for creating a server-side listening socket, and accepting new connections from connecting clients. 

### HttpConnection
HttpConnection handles the reading in of an HTTP request over a network connection, parsing such requests into an object, and also the writing of responses back over the connection. This will largely deal with string manipulation to read and parse the HTTP requests.

### HttpUtils
HttpUtils provides some utilities that we will need for our search server. In particular, there are two functions that we implemented for making sure that our server handles some security concerns. The function is_path_safe is used to make sure that anyone using the server can only access files under the specified static files directory. If we don’t implement and use the function, then it is possible that an attacker could request any file on our computer that they would like with something called a directory traversal attack.

The other function escape_html is used to prevent a “cross-site scripting” flaw. See this for background if you’re curious: http://en.wikipedia.org/wiki/Cross-site_scripting

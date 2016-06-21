# tcp-fileserver
C++ implementation of TCP File Server

﻿Use TCP Protocol in order to create a file server which will have 2 commands GET, INFO

GET provides all files in the folder that the server was started

GET will also provide file contents to the client // see example run below

INFO - provides current date and time of the server machine


Test was completed via telnet from Terminal window using the command - telnet localhost 9001 - where 9001 is the port that the server was started

Server command line arguments are the folder that it is to be started with and the port number of choice example is ./tcp_server 9001 /home/alex/Desktop

NOTE: GET will not display hidden files, this functionality can be enabled from within the code!

Example Run is located in the Example_Run file

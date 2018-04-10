# socket_chat

This project consists of two parts: server and client. Clients can leave messages for other clients through the server, and they can also chat directly with each other without the server.

There are currently two users coded in the server code for testing purpose:
- bob, password 1234
- alice, password 1234

To run the server:
  - go into the server folder <code>src/server</code>
  - run script <code>compServer</code> <br/>
  <code>sh compServer</code>
  - run the server program <br/>
  <code>./server 'port number'</code> , e.g. <code>./server 5432</code><br/>
  or <code>./server</code> if you don't have a specific port in mind. The program uses port 8000 as default port.
  
To clear all compiled file of server:
  - go into the server folder <code>src/server</code>
  - run script <code>clean</code> <br/>
  <code>sh clean</code>
 
 To run the client:
  - go into the client folder <code>src/client</code>
  - run script <code>compClient</code> <br/>
  <code>sh compClient</code>
  - run the client program <br/>
  <code>./client</code>
  
To clear all compiled file of client:
  - go into the client folder <code>src/client</code>
  - run script <code>clean</code> <br/>
  <code>sh clean</code>

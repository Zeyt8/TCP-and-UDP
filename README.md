# TCP and UDP - Communication Protocols - Project 2

A pdf with the detailed description can be found in the repo.

## Server

The server handles messages in 4 ways:

1. If they come from the socket reserved for connections from the client.

	The socket is a stream that listens for connection requests. It accepts the request, creates a new socket for that client, and prepares to receive a message with the client's ID.

2. If they come from the socket reserved for messages from UDP clients.

	The message has the source IP, source port, and '\r' added to the end for framing. It checks if any client is subscribed to that topic. If the client is connected, it sends the message, if not and sf is set to 1 for that topic, it adds the message to that client's message queue.

3. If they come from the console.

	If the message is "exit", the server closes and sends a message to all clients to close.

4. Otherwise.

	If an ID message is expected, it reads the message. It checks if there is another client with the same ID. If there is and it is not connected, it connects and sends all stored messages in the queue. If there is and it is connected, it does not allow the connection. If there is not, it creates a new client and adds it to the list of clients.

	If an ID message is not expected, it checks if the message is a subscribe or unsubscribe type. In the case of subscribe, it reads the topic and sf and adds them to the relevant client. In the case of unsubscribe, it removes the necessary topic and sf.

## Subscriber

The subscriber handles messages in 2 ways:

1. Messages from the console.

	Subscribe sends a subscribe message to the server. The message has the first byte 's' for identification, followed by 50 bytes with the topic, 1 byte with the value of sf, and 1 byte of '\0'.

	Unsubscribe sends an unsubscribe message to the server. The message has the first byte 'u', followed by 50 bytes with the topic and 1 byte of '\0'.

	Exit closes the subscriber.

2. Messages from the server.

	If the receiving socket is active, the subscriber receives data in a while loop until a complete message has been received. This is checked by the presence of the '\r' character marking the end of a message. Upon receiving data, the pointer is incremented by the number of bytes received, i.e. to the position where it should be added in the reading buffer. When a complete message is detected, it is copied to another buffer, the beginning of the next message is moved to the beginning of the reading buffer, and the pointer is decremented by the length of a message.

	It then moves on to processing the message. It checks the type of message. In the case of numeric, it reads the number from the buffer and modifies it so that it has the correct byte order. It does other calculations depending on the case and displays it. The IP and port of the source are taken from the end of the message, their position in the message is explained in Server (2.).

#

The implementation for ue_vector is taken from here: https://codereview.stackexchange.com/questions/253173/generic-vector-implemented-in-c-language

List and queue are taken from the skeleton of task 1.

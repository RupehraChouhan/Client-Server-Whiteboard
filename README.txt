Instructions to run CLIENT USER INTERFACE
1. wbc379 hostname portnumber [keyfile] : If you are using my server then
	use 127.0.0.1 or localhost as the hostname
	
2.COMMANDS: Answer the command and hit enter

	Command 1: 
	"Request (?), update (@) or exit :"
		-> Type ? to query the entry, @ to update and type "exit" without
		-> quotes to exit the client

	Command 2: 
	"Entry number > 0:"
		-> This is the entry number in the whiteboard. Input must be greater than 0

	Command 3: (If the client selects @ from command 1)
	"Plaintext or encryted (p or c): "
		-> Type "p" if the entry is supposed to be in plain text and "c"
		-> if you want to encrypt the entry

	Command 4:
	"How long is the message? "
		-> Length of the message that you want to update

	Command 5:
	"Enter the message: "
		-> Enter message of the specified length
		
3. Press CTRL-C anytime or type "exit" in command 1 to exit
